/*
 * Load_fc.cpp
 * -------------
 * Purpose: Future Composer 1.0 - 1.4 module loader
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"
#include "InstrumentSynth.h"

OPENMPT_NAMESPACE_BEGIN

struct FCPlaylistEntry
{
	struct Entry
	{
		uint8 pattern;
		int8 noteTranspose;
		int8 instrTranspose;
	};

	std::array<Entry, 4> channels;
	uint8 speed;
};

MPT_BINARY_STRUCT(FCPlaylistEntry, 13)


struct FCSampleInfo
{
	uint16be length;      // In words
	uint16be loopStart;   // In bytes
	uint16be loopLength;  // In words

	void ConvertToMPT(ModSample &mptSmp, FileReader &file, bool isFC14, bool loadSampleData) const
	{
		mptSmp.Initialize(MOD_TYPE_MOD);
		mptSmp.nLength = length * 2;
		if(isFC14 && mptSmp.nLength)
			mptSmp.nLength += 2;
		mptSmp.nLoopStart = loopStart;
		mptSmp.nLoopEnd = mptSmp.nLoopStart + loopLength * 2;
		mptSmp.uFlags.set(CHN_LOOP, loopLength > 1);

		// Fix for axel foley remix.smod (loop extends into next sample)
		auto nextSampleStart = file.GetPosition() + mptSmp.nLength;
		if(mptSmp.uFlags[CHN_LOOP] && mptSmp.nLoopEnd > mptSmp.nLength)
			mptSmp.nLength = mptSmp.nLoopEnd;
		if(loadSampleData)
		{
			SampleIO{SampleIO::_8bit, SampleIO::mono, SampleIO::bigEndian, SampleIO::signedPCM}
				.ReadSample(mptSmp, file);
		}
		file.Seek(nextSampleStart);
	}
};

MPT_BINARY_STRUCT(FCSampleInfo, 6)


struct FCFileHeader
{
	char     magic[4];      // "SMOD" (Future Composer 1.0 - 1.3) or "FC14" (Future Composer 1.4)
	uint32be sequenceSize;  // All sizes in bytes
	uint32be patternsOffset;
	uint32be patternsSize;
	uint32be freqSequenceOffset;
	uint32be freqSequenceSize;
	uint32be volSequenceOffset;
	uint32be volSequenceSize;
	uint32be sampleDataOffset;
	uint32be waveTableOffset;  // or sample data size if version < 1.4
	
	std::array<FCSampleInfo, 10> sampleInfo;

	bool IsValid() const
	{
		if(memcmp(magic, "SMOD", 4) && memcmp(magic, "FC14", 4))
			return false;

		static constexpr uint32 MAX_FC_SIZE = 0x8'0000;  // According to manual: Sample memory allocated = 100,000 bytes
		if(sequenceSize % sizeof(FCPlaylistEntry) > 1    // Some files have a mysterious extra byte
		   || sequenceSize < sizeof(FCPlaylistEntry)      || sequenceSize > 256 * 13
		   || patternsSize % 64u     || !patternsSize     || patternsSize > 64 * 256     || patternsOffset > MAX_FC_SIZE
		   || freqSequenceSize % 64u || !freqSequenceSize || freqSequenceSize > 64 * 256 || freqSequenceOffset > MAX_FC_SIZE
		   || volSequenceSize % 64u  || !volSequenceSize  || volSequenceSize > 64 * 256  || volSequenceOffset > MAX_FC_SIZE
		   || sampleDataOffset > MAX_FC_SIZE
		   || waveTableOffset > MAX_FC_SIZE)
			return false;

		return true;
	}

	bool IsFC14() const
	{
		return !memcmp(magic, "FC14", 4);
	}

	ORDERINDEX NumOrders() const
	{
		return static_cast<ORDERINDEX>(sequenceSize / sizeof(FCPlaylistEntry));
	}

	uint32 GetHeaderMinimumAdditionalSize() const
	{
		return sequenceSize + (IsFC14() ? 80 : 0);
	}
};

MPT_BINARY_STRUCT(FCFileHeader, 100)


static void TranslateFCScript(InstrumentSynth::Events &events, const mpt::span<const uint8> script, const bool isFC14, const uint16 startSequence = uint16_max)
{
	FileReader file{script};
	const bool isVolume = startSequence > 255;
	const uint8 volScriptSpeed = (script[0] > 0) ? script[0] : uint8_max;
	if(isVolume)
		events.push_back(InstrumentSynth::Event::SetStepSpeed(volScriptSpeed, true));

	std::vector<uint8> sequencesToParse(1, static_cast<uint8>(isVolume ? 0 : startSequence));
	std::bitset<256> parsedSequences;
	parsedSequences.set(sequencesToParse.back());

	std::map<uint16, uint16> entryFromByte;
	while(!sequencesToParse.empty())
	{
		const uint16 currentSequenceOffset = sequencesToParse.back() * 64u;
		sequencesToParse.pop_back();
		file.Seek(currentSequenceOffset);
		if(isVolume)
			file.Skip(5);

		// Due to an underflow bug in the volume command E0, it can jump up to 260 bytes from the start of the chosen volume sequence.
		// Luckily this is not possible with frequency sequences - as multiple frequency sequences can be chained using command E7,
		// not having to care for that bug there makes the book-keeping a bit easier for us, as jumps will always be "local" within the currently selected sequence,
		// and a jump target address cannot belong to two parsed sequences.
		const uint16 maxScriptJumpPos = static_cast<uint16>(currentSequenceOffset + (isVolume ? 260 : 63));
		uint16 maxJump = 0;
		bool nextByteIsPitch = false;
		while(file.CanRead(1))
		{
			uint16 scriptPos = static_cast<uint16>(file.GetPosition());
			if(scriptPos <= maxScriptJumpPos)
				entryFromByte[scriptPos] = static_cast<uint16>(events.size());
			const uint8 b = file.ReadUint8();
			
			// After several pitch Ex commands, the next byte is always interpreted as Set Pitch
			if(nextByteIsPitch)
			{
				events.push_back(InstrumentSynth::Event::FC_SetPitch(b));
				nextByteIsPitch = false;
				continue;
			}

			const auto volumeCommand = InstrumentSynth::Event::MED_SetVolume(static_cast<uint8>(std::min(b & 0x7F, 64)));
			switch(b)
			{
			case 0xE0:  // Loop (position)
				{
					uint16 target = file.ReadUint8() & 0x3F;
					if(isVolume && target < 5)  // Volume sequence offset is relative to first volume command at offset 5, so FC subtracts 5 from the jump target and happily underflows the byte value
						target += 256;
					target += currentSequenceOffset;
					if(!isVolume && target < script.size() && (script[target] == 0xE0 || script[target] == 0xE1))
					{
						// The first byte that is executed after an E0 jump is never interpreted as command E0 or E1.
						events.push_back(InstrumentSynth::Event::FC_SetPitch(script[target]));
						target++;
						if(target < script.size() && script[target - 1] == 0xE0)
						{
							events.push_back(InstrumentSynth::Event::FC_SetPitch(script[target]));
							target++;
						}
					}
					maxJump = std::max(maxJump, target);
					events.push_back(InstrumentSynth::Event::Jump(target));
				}
				break;
			case 0xE1:  // End (none)
				events.push_back(InstrumentSynth::Event::StopScript());
				break;
			case 0xE2:  // Set waveform (waveform)
				if(!isVolume)
					events.push_back(InstrumentSynth::Event::MED_JumpScript(1, 0));
				[[fallthrough]];
			case 0xE4:  // Change waveform (waveform)
				if(isVolume)
					events.push_back(volumeCommand);
				else
					events.push_back(InstrumentSynth::Event::FC_SetWaveform(b, file.ReadUint8(), 0));
				break;
			case 0xE3:  // Set new vibrato (speed, amplitude)
				if(isVolume)
				{
					events.push_back(volumeCommand);
				} else
				{
					const auto [speed, depth] = file.ReadArray<uint8, 2>();
					events.push_back(InstrumentSynth::Event::FC_SetVibrato(speed, depth, 0));
				}
				break;
			case 0xE7:  // Jump to freq. sequence (sequence)
				if(isVolume)
				{
					events.push_back(volumeCommand);
				} else
				{
					uint8 sequence = file.ReadUint8();
					events.push_back(InstrumentSynth::Event::Jump(sequence * 64u));
					if(!parsedSequences[sequence])
					{
						parsedSequences.set(sequence);
						sequencesToParse.push_back(sequence);
					}
				}
				break;
			case 0xE8:  // Sustain (time)
				{
					const uint8 delay = file.ReadUint8();
					if(isVolume && volScriptSpeed > 1)
					{
						events.push_back(InstrumentSynth::Event::SetStepSpeed(1, true));
						events.push_back(InstrumentSynth::Event::Delay(static_cast<uint16>(delay + volScriptSpeed - 2)));
						events.push_back(InstrumentSynth::Event::SetStepSpeed(volScriptSpeed, true));
					} else if(delay)
					{
						events.push_back(InstrumentSynth::Event::Delay(delay - 1));
					}
				}
				break;
			case 0xE9:  // Set waveform (waveform, number)
				if(isVolume)
				{
					events.push_back(volumeCommand);
				} else if(isFC14)
				{
					const auto [waveform, subSample] = file.ReadArray<uint8, 2>();
					events.push_back(InstrumentSynth::Event::MED_JumpScript(1, 0));
					events.push_back(InstrumentSynth::Event::FC_SetWaveform(b, subSample, waveform));
				} else
				{
					events.push_back(InstrumentSynth::Event::FC_SetPitch(b));
				}
				break;
			case 0xEA:  // Volume slide / Pitch bend (step, time)
				if(isFC14)
				{
					const auto [speed, time] = file.ReadArray<uint8, 2>();
					if(isVolume)
						events.push_back(InstrumentSynth::Event::FC_VolumeSlide(speed, time));
					else
						events.push_back(InstrumentSynth::Event::FC_PitchSlide(speed, time));
					break;
				}
				[[fallthrough]];
			default:
				if(isVolume)
					events.push_back(volumeCommand);
				else
					events.push_back(InstrumentSynth::Event::FC_SetPitch(b));
				break;
			}
			if(!isVolume)
				nextByteIsPitch = (b >= 0xE2 && b <= 0xE4) || (isFC14 && (b == 0xE9 || b == 0xEA));

			// If a sequence doesn't end with an E0/E1/E7 command, execution will continue in the next sequence.
			// In the general case, we cannot stop translating the script at an E0/E1 command because we might jump beyond it with an E0 command.
			// However, E0 commands cannot jump beyond the end of the initial sequence (modulo underflow, see above), so we can stop translating additonal sequences when we hit command E0/E1 there.
			if((b == 0xE0 || b == 0xE1 || (b == 0xE7 && !isVolume)) && (maxJump <= scriptPos || scriptPos >= maxScriptJumpPos))
				break;
		}
	}
	
	for(auto &event : events)
	{
		event.FixupJumpTarget(entryFromByte);
	}
}


CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderFC(MemoryFileReader file, const uint64 *pfilesize)
{
	FCFileHeader fileHeader;
	if(!file.ReadStruct(fileHeader))
		return ProbeWantMoreData;
	if(!fileHeader.IsValid())
		return ProbeFailure;
	return ProbeAdditionalSize(file, pfilesize, fileHeader.GetHeaderMinimumAdditionalSize());
}


bool CSoundFile::ReadFC(FileReader &file, ModLoadingFlags loadFlags)
{
	FCFileHeader fileHeader;

	file.Rewind();
	if(!file.ReadStruct(fileHeader) || !fileHeader.IsValid())
		return false;
	if(!file.CanRead(fileHeader.GetHeaderMinimumAdditionalSize()))
		return false;
	if(loadFlags == onlyVerifyHeader)
		return true;

	const bool isFC14 = fileHeader.IsFC14();
	InitializeGlobals(MOD_TYPE_MOD, 4);
	SetupMODPanning(true);
	m_SongFlags.set(SONG_IMPORTED | SONG_ISAMIGA);
	m_nSamples = isFC14 ? 90 : 57;
	m_nInstruments = static_cast<INSTRUMENTINDEX>(fileHeader.volSequenceSize / 64u + 1);
	m_playBehaviour.set(kMODSampleSwap);
	m_playBehaviour.set(kApplyUpperPeriodLimit);
	m_nMinPeriod = 113 * 4;
	m_nMaxPeriod = 3424 * 4;

	m_modFormat.formatName = isFC14 ? UL_("Future Composer 1.4") : UL_("Future Composer 1.0 - 1.3");
	m_modFormat.type = isFC14 ? UL_("fc") : UL_("smod");
	m_modFormat.madeWithTracker = m_modFormat.formatName;
	m_modFormat.charset = mpt::Charset::Amiga_no_C1;  // No strings in this format...

	std::array<uint8, 80> waveTableLengths;
	if(isFC14)
		file.ReadArray(waveTableLengths);

	const ORDERINDEX numOrders = fileHeader.NumOrders();
	std::vector<FCPlaylistEntry> orderData;
	file.ReadVector(orderData, numOrders);

	std::vector<std::array<uint8, 2>> patternData(fileHeader.patternsSize / 2u);
	file.Seek(fileHeader.patternsOffset);
	if(!file.ReadVector(patternData, fileHeader.patternsSize / 2u))
		return false;
	
	Order().SetDefaultSpeed(3);
	Order().resize(numOrders);
	if(loadFlags & loadPatternData)
		Patterns.ResizeArray(numOrders);
	std::array<uint8, 4> prevNote = {{}};
	for(ORDERINDEX ord = 0; ord < numOrders; ord++)
	{
		Order()[ord] = ord;
		static constexpr uint16 numRows = 32;
		if(!(loadFlags & loadPatternData) || !Patterns.Insert(ord, numRows))
			continue;
		ROWINDEX lastRow = numRows;
		for(CHANNELINDEX chn = 0; chn < 4; chn++)
		{
			const auto &chnInfo = orderData[ord].channels[chn];
			const uint16 patternOffset = chnInfo.pattern * numRows;
			if(patternOffset >= patternData.size())
				continue;
			ModCommand *m = Patterns[ord].GetpModCommand(0, chn);
			const auto &pattern = mpt::as_span(patternData).subspan(patternOffset);
			for(ROWINDEX row = 0; row < numRows; row++, m += GetNumChannels())
			{
				const auto p = pattern[row];
				// Channels should be broken independently, but no tune in the wild relies on it, I think.
				// (Vim's blue-funk has inconsistent pattern lengths in the very last order item but that's all I could find)
				if(p[0] == 0x49)
					lastRow = std::min(lastRow, row);
				
				if(p[0] > 0 && p[0] != 0x49)
				{
					prevNote[chn] = p[0];
					m->note = NOTE_MIN + ((chnInfo.noteTranspose + p[0]) & 0x7F);
					if(int instr = (p[1] & 0x3F) + chnInfo.instrTranspose + 1; instr >= 1 && instr <= m_nInstruments)
						m->instr = static_cast<ModCommand::INSTR>(instr);
					else
						m->instr = static_cast<ModCommand::INSTR>(m_nInstruments);
				} else if(row == 0 && ord > 0 && orderData[ord - 1].channels[chn].noteTranspose != chnInfo.noteTranspose && prevNote[chn] > 0)
				{
					m->note = NOTE_MIN + ((chnInfo.noteTranspose + prevNote[chn]) & 0x7F);
					if(p[1] & 0xC0)
						m->SetVolumeCommand(VOLCMD_TONEPORTAMENTO, 9);
					else
						m->SetEffectCommand(CMD_TONEPORTA_DURATION, 0);
				}
				if(p[1] & 0xC0)
					m->SetEffectCommand(CMD_AUTO_PORTAMENTO_FC, 0);
				if(p[1] & 0x80)
				{
					const uint8 data = ((row + 1) < pattern.size()) ? pattern[row + 1][1] : uint8(0);
					int8 param = data & 0x1F;
					if(!isFC14)
						param *= 2;
					if(data > 0x1F)
						param = -param;
					m->param = static_cast<uint8>(param);
				}
			}
		}
		if(orderData[ord].speed)
			Patterns[ord].WriteEffect(EffectWriter(CMD_SPEED, orderData[ord].speed).RetryNextRow());
		if(lastRow < numRows)
			Patterns[ord].WriteEffect(EffectWriter(CMD_PATTERNBREAK, 0).Row(std::max(lastRow, ROWINDEX(1)) - 1).RetryNextRow());
	}

	std::vector<uint8> freqSequences, volSequences;
	freqSequences.reserve(64 * 256);
	if(!file.Seek(fileHeader.freqSequenceOffset) || !file.ReadVector(freqSequences, fileHeader.freqSequenceSize))
		return false;
	freqSequences.resize(64 * 256);
	volSequences.reserve(fileHeader.volSequenceSize + 8);
	if(!file.Seek(fileHeader.volSequenceOffset) || !file.ReadVector(volSequences, fileHeader.volSequenceSize))
		return false;

	// Add an empty instrument for out-of-range instrument numbers
	static constexpr std::array<uint8, 8> EmptyInstr = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE1};
	volSequences.insert(volSequences.end(), EmptyInstr.begin(), EmptyInstr.end());

	for(INSTRUMENTINDEX ins = 1; ins <= m_nInstruments; ins++)
	{
		const auto volSeq = mpt::as_span(volSequences).subspan((ins - 1) * 64u);
		const auto freqSeq = mpt::as_span(freqSequences).subspan(volSeq[1] * 64u);
		uint8 defaultSample = freqSeq[1];
		if(freqSeq[0] == 0xE9)
			defaultSample = static_cast<uint8>(90 + defaultSample * 10 + freqSeq[2]);

		ModInstrument *instr = AllocateInstrument(ins, defaultSample + 1);
		if(!instr)
			return false;

		static_assert(NOTE_MAX - NOTE_MIN + 1 >= 128, "Need at least a note range of 128 for correct emulation of note wrap-around logic in Future Composer");
		for(uint8 note = 0; note < static_cast<uint8>(instr->NoteMap.size()); note++)
		{
			if(note < 48)
				instr->NoteMap[note] = note + NOTE_MIDDLEC - 24;
			else if(note < 60 || note >= 120)
				instr->NoteMap[note] = NOTE_MIDDLEC + 23;
			else
				instr->NoteMap[note] = note + NOTE_MIDDLEC - 36 - 60;
		}

		instr->synth.m_scripts.resize(2);
		instr->synth.m_scripts[0].push_back(InstrumentSynth::Event::FC_SetVibrato(volSeq[2], volSeq[3], volSeq[4]));
		TranslateFCScript(instr->synth.m_scripts[0], mpt::as_span(freqSequences), isFC14, volSeq[1]);
		TranslateFCScript(instr->synth.m_scripts[1], volSeq, isFC14);
	}

	if(!file.Seek(fileHeader.sampleDataOffset))
		return false;
	for(SAMPLEINDEX smp = 0; smp < 10; smp++)
	{
		if(!fileHeader.sampleInfo[smp].length)
			continue;

		if(isFC14 && file.ReadMagic("SSMP"))
		{
			m_nSamples = 190;
			FileReader sampleHeaders = file.ReadChunk(160);
			for(SAMPLEINDEX subSmp = 0; subSmp < 10; subSmp++)
			{
				FCSampleInfo sampleInfo;
				sampleHeaders.Skip(4);
				sampleHeaders.Read(sampleInfo);
				sampleHeaders.Skip(6);

				sampleInfo.ConvertToMPT(Samples[91 + (smp * 10) + subSmp], file, true, (loadFlags & loadSampleData) != 0);
			}
		} else
		{
			fileHeader.sampleInfo[smp].ConvertToMPT(Samples[smp + 1], file, isFC14, (loadFlags& loadSampleData) != 0);
		}
	}

	if(!(loadFlags & loadSampleData))
		return true;

	static constexpr uint8 SampleLengths[] =
	{
		16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
		16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
		8, 8, 8, 8, 8, 8, 8, 8, 16, 8, 16, 16, 8, 8, 24,  // Future Composer 1.4 imports the last sample with a length of 32 instead of 48, causing some older FC files to be detuned.
	};

	static constexpr uint8 SampleData[] =
	{
		0xC0, 0xC0, 0xD0, 0xD8, 0xE0, 0xE8, 0xF0, 0xF8, 0x00, 0xF8, 0xF0, 0xE8, 0xE0, 0xD8, 0xD0, 0xC8, 0x3F, 0x37, 0x2F, 0x27, 0x1F, 0x17, 0x0F, 0x07, 0xFF, 0x07, 0x0F, 0x17, 0x1F, 0x27, 0x2F, 0x37,
		0xC0, 0xC0, 0xD0, 0xD8, 0xE0, 0xE8, 0xF0, 0xF8, 0x00, 0xF8, 0xF0, 0xE8, 0xE0, 0xD8, 0xD0, 0xC8, 0xC0, 0x37, 0x2F, 0x27, 0x1F, 0x17, 0x0F, 0x07, 0xFF, 0x07, 0x0F, 0x17, 0x1F, 0x27, 0x2F, 0x37,
		0xC0, 0xC0, 0xD0, 0xD8, 0xE0, 0xE8, 0xF0, 0xF8, 0x00, 0xF8, 0xF0, 0xE8, 0xE0, 0xD8, 0xD0, 0xC8, 0xC0, 0xB8, 0x2F, 0x27, 0x1F, 0x17, 0x0F, 0x07, 0xFF, 0x07, 0x0F, 0x17, 0x1F, 0x27, 0x2F, 0x37,
		0xC0, 0xC0, 0xD0, 0xD8, 0xE0, 0xE8, 0xF0, 0xF8, 0x00, 0xF8, 0xF0, 0xE8, 0xE0, 0xD8, 0xD0, 0xC8, 0xC0, 0xB8, 0xB0, 0x27, 0x1F, 0x17, 0x0F, 0x07, 0xFF, 0x07, 0x0F, 0x17, 0x1F, 0x27, 0x2F, 0x37,
		0xC0, 0xC0, 0xD0, 0xD8, 0xE0, 0xE8, 0xF0, 0xF8, 0x00, 0xF8, 0xF0, 0xE8, 0xE0, 0xD8, 0xD0, 0xC8, 0xC0, 0xB8, 0xB0, 0xA8, 0x1F, 0x17, 0x0F, 0x07, 0xFF, 0x07, 0x0F, 0x17, 0x1F, 0x27, 0x2F, 0x37,
		0xC0, 0xC0, 0xD0, 0xD8, 0xE0, 0xE8, 0xF0, 0xF8, 0x00, 0xF8, 0xF0, 0xE8, 0xE0, 0xD8, 0xD0, 0xC8, 0xC0, 0xB8, 0xB0, 0xA8, 0xA0, 0x17, 0x0F, 0x07, 0xFF, 0x07, 0x0F, 0x17, 0x1F, 0x27, 0x2F, 0x37,
		0xC0, 0xC0, 0xD0, 0xD8, 0xE0, 0xE8, 0xF0, 0xF8, 0x00, 0xF8, 0xF0, 0xE8, 0xE0, 0xD8, 0xD0, 0xC8, 0xC0, 0xB8, 0xB0, 0xA8, 0xA0, 0x98, 0x0F, 0x07, 0xFF, 0x07, 0x0F, 0x17, 0x1F, 0x27, 0x2F, 0x37,
		0xC0, 0xC0, 0xD0, 0xD8, 0xE0, 0xE8, 0xF0, 0xF8, 0x00, 0xF8, 0xF0, 0xE8, 0xE0, 0xD8, 0xD0, 0xC8, 0xC0, 0xB8, 0xB0, 0xA8, 0xA0, 0x98, 0x90, 0x07, 0xFF, 0x07, 0x0F, 0x17, 0x1F, 0x27, 0x2F, 0x37,
		0xC0, 0xC0, 0xD0, 0xD8, 0xE0, 0xE8, 0xF0, 0xF8, 0x00, 0xF8, 0xF0, 0xE8, 0xE0, 0xD8, 0xD0, 0xC8, 0xC0, 0xB8, 0xB0, 0xA8, 0xA0, 0x98, 0x90, 0x88, 0xFF, 0x07, 0x0F, 0x17, 0x1F, 0x27, 0x2F, 0x37,
		0xC0, 0xC0, 0xD0, 0xD8, 0xE0, 0xE8, 0xF0, 0xF8, 0x00, 0xF8, 0xF0, 0xE8, 0xE0, 0xD8, 0xD0, 0xC8, 0xC0, 0xB8, 0xB0, 0xA8, 0xA0, 0x98, 0x90, 0x88, 0x80, 0x07, 0x0F, 0x17, 0x1F, 0x27, 0x2F, 0x37,
		0xC0, 0xC0, 0xD0, 0xD8, 0xE0, 0xE8, 0xF0, 0xF8, 0x00, 0xF8, 0xF0, 0xE8, 0xE0, 0xD8, 0xD0, 0xC8, 0xC0, 0xB8, 0xB0, 0xA8, 0xA0, 0x98, 0x90, 0x88, 0x80, 0x88, 0x0F, 0x17, 0x1F, 0x27, 0x2F, 0x37,
		0xC0, 0xC0, 0xD0, 0xD8, 0xE0, 0xE8, 0xF0, 0xF8, 0x00, 0xF8, 0xF0, 0xE8, 0xE0, 0xD8, 0xD0, 0xC8, 0xC0, 0xB8, 0xB0, 0xA8, 0xA0, 0x98, 0x90, 0x88, 0x80, 0x88, 0x90, 0x17, 0x1F, 0x27, 0x2F, 0x37,
		0xC0, 0xC0, 0xD0, 0xD8, 0xE0, 0xE8, 0xF0, 0xF8, 0x00, 0xF8, 0xF0, 0xE8, 0xE0, 0xD8, 0xD0, 0xC8, 0xC0, 0xB8, 0xB0, 0xA8, 0xA0, 0x98, 0x90, 0x88, 0x80, 0x88, 0x90, 0x98, 0x1F, 0x27, 0x2F, 0x37,
		0xC0, 0xC0, 0xD0, 0xD8, 0xE0, 0xE8, 0xF0, 0xF8, 0x00, 0xF8, 0xF0, 0xE8, 0xE0, 0xD8, 0xD0, 0xC8, 0xC0, 0xB8, 0xB0, 0xA8, 0xA0, 0x98, 0x90, 0x88, 0x80, 0x88, 0x90, 0x98, 0xA0, 0x27, 0x2F, 0x37,
		0xC0, 0xC0, 0xD0, 0xD8, 0xE0, 0xE8, 0xF0, 0xF8, 0x00, 0xF8, 0xF0, 0xE8, 0xE0, 0xD8, 0xD0, 0xC8, 0xC0, 0xB8, 0xB0, 0xA8, 0xA0, 0x98, 0x90, 0x88, 0x80, 0x88, 0x90, 0x98, 0xA0, 0xA8, 0x2F, 0x37,
		0xC0, 0xC0, 0xD0, 0xD8, 0xE0, 0xE8, 0xF0, 0xF8, 0x00, 0xF8, 0xF0, 0xE8, 0xE0, 0xD8, 0xD0, 0xC8, 0xC0, 0xB8, 0xB0, 0xA8, 0xA0, 0x98, 0x90, 0x88, 0x80, 0x88, 0x90, 0x98, 0xA0, 0xA8, 0xB0, 0x37,
		0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
		0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
		0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
		0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
		0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
		0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
		0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
		0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
		0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
		0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
		0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
		0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
		0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x7F, 0x7F, 0x7F, 0x7F,
		0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x7F, 0x7F, 0x7F,
		0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x7F, 0x7F,
		0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x7F,
		0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
		0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
		0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
		0x80, 0x80, 0x80, 0x80, 0x80, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
		0x80, 0x80, 0x80, 0x80, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
		0x80, 0x80, 0x80, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
		0x80, 0x80, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
		0x80, 0x80, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
		0x80, 0x80, 0x90, 0x98, 0xA0, 0xA8, 0xB0, 0xB8, 0xC0, 0xC8, 0xD0, 0xD8, 0xE0, 0xE8, 0xF0, 0xF8, 0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38, 0x40, 0x48, 0x50, 0x58, 0x60, 0x68, 0x70, 0x7F,
		0x80, 0x80, 0xA0, 0xB0, 0xC0, 0xD0, 0xE0, 0xF0, 0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70,
		0x45, 0x45, 0x79, 0x7D, 0x7A, 0x77, 0x70, 0x66, 0x61, 0x58, 0x53, 0x4D, 0x2C, 0x20, 0x18, 0x12, 0x04, 0xDB, 0xD3, 0xCD, 0xC6, 0xBC, 0xB5, 0xAE, 0xA8, 0xA3, 0x9D, 0x99, 0x93, 0x8E, 0x8B, 0x8A,
		0x45, 0x45, 0x79, 0x7D, 0x7A, 0x77, 0x70, 0x66, 0x5B, 0x4B, 0x43, 0x37, 0x2C, 0x20, 0x18, 0x12, 0x04, 0xF8, 0xE8, 0xDB, 0xCF, 0xC6, 0xBE, 0xB0, 0xA8, 0xA4, 0x9E, 0x9A, 0x95, 0x94, 0x8D, 0x83,
		0x00, 0x00, 0x40, 0x60, 0x7F, 0x60, 0x40, 0x20, 0x00, 0xE0, 0xC0, 0xA0, 0x80, 0xA0, 0xC0, 0xE0,
		0x00, 0x00, 0x40, 0x60, 0x7F, 0x60, 0x40, 0x20, 0x00, 0xE0, 0xC0, 0xA0, 0x80, 0xA0, 0xC0, 0xE0,
		0x80, 0x80, 0x90, 0x98, 0xA0, 0xA8, 0xB0, 0xB8, 0xC0, 0xC8, 0xD0, 0xD8, 0xE0, 0xE8, 0xF0, 0xF8, 0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38, 0x40, 0x48, 0x50, 0x58, 0x60, 0x68, 0x70, 0x7F, 0x80, 0x80, 0xA0, 0xB0, 0xC0, 0xD0, 0xE0, 0xF0, 0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70,
	};

	if(isFC14 && !file.Seek(fileHeader.waveTableOffset))
		return false;

	FileReader smpFile{mpt::as_span(SampleData)};
	const mpt::span<const uint8> sampleLengths = isFC14 ? mpt::as_span(waveTableLengths) : mpt::as_span(SampleLengths);
	SampleIO sampleIO{SampleIO::_8bit, SampleIO::mono, SampleIO::bigEndian, SampleIO::signedPCM};
	for(SAMPLEINDEX smp = 0; smp < sampleLengths.size(); smp++)
	{
		ModSample &mptSmp = Samples[smp + 11];
		mptSmp.Initialize(MOD_TYPE_MOD);
		mptSmp.nLength = sampleLengths[smp] * 2u;
		mptSmp.nLoopStart = 0;
		mptSmp.nLoopEnd = mptSmp.nLength;
		mptSmp.uFlags.set(CHN_LOOP);
		sampleIO.ReadSample(mptSmp, isFC14 ? file : smpFile);
	}

	return true;
}


OPENMPT_NAMESPACE_END
