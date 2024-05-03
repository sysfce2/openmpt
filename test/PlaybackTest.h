/*
 * PlaybackTest.h
 * --------------
 * Purpose: Tools for verifying correct playback of modules
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"
#include "../common/FileReaderFwd.h"

#include <iosfwd>

OPENMPT_NAMESPACE_BEGIN

struct PlaybackTestData;
class CSoundFile;

class PlaybackTest
{
public:
	explicit PlaybackTest(FileReader file) noexcept(false);
	explicit PlaybackTest(PlaybackTestData &&testData);
	PlaybackTest(PlaybackTest &&other) noexcept;
	PlaybackTest(const PlaybackTest &) = delete;
	~PlaybackTest();

	PlaybackTest& operator=(PlaybackTest &&other) noexcept;
	PlaybackTest& operator=(const PlaybackTest &) = delete;

	void Deserialize(FileReader file) noexcept(false);
	void Serialize(std::ostream &output, const mpt::ustring &filename) const noexcept(false);
	void ToTSV(std::ostream &output) const noexcept(false);

	std::vector<mpt::ustring> Compare(CSoundFile &sndFile) const;
	std::vector<mpt::ustring> Compare(const PlaybackTest &otherTest) const;

private:
	std::unique_ptr<PlaybackTestData> m_testData;
};

OPENMPT_NAMESPACE_END
