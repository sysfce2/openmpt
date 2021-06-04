/*
 * libopenmpt_plugin_gui.hpp
 * -------------------------
 * Purpose: libopenmpt plugin GUI
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#ifndef LIBOPENMPT_PLUGIN_GUI_HPP
#define LIBOPENMPT_PLUGIN_GUI_HPP

#include "libopenmpt_plugin_settings.hpp"

#define NOMINMAX
#include <windows.h>

#include <string>


namespace libopenmpt {
namespace plugin {

#if defined(MPT_WITH_MFC)

void DllMainAttach();
void DllMainDetach();

#endif // MPT_WITH_MFC

void gui_edit_settings( libopenmpt_settings * s, HWND parent, std::wstring title );

void gui_show_file_info( HWND parent, std::wstring title, std::wstring info );


} // namespace plugin
} // namespace libopenmpt


#endif // LIBOPENMPT_PLUGIN_GUI_HPP
