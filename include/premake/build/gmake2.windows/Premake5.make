# Alternative GNU Make project makefile autogenerated by Premake

ifndef config
  config=release_win32
endif

ifndef verbose
  SILENT = @
endif

.PHONY: clean prebuild

SHELLTYPE := posix
ifeq (.exe,$(findstring .exe,$(ComSpec)))
	SHELLTYPE := msdos
endif

# Configurations
# #############################################

RESCOMP = windres
INCLUDES += -I../../contrib/lua/src -I../../contrib/luashim -I../../contrib/zlib -I../../contrib/libzip -I../../contrib/curl/include
FORCE_INCLUDE +=
ALL_CPPFLAGS += $(CPPFLAGS) -MMD -MP $(DEFINES) $(INCLUDES)
ALL_RESFLAGS += $(RESFLAGS) $(DEFINES) $(INCLUDES)
LINKCMD = $(CC) -o "$@" $(OBJECTS) $(RESOURCES) $(ALL_LDFLAGS) $(LIBS)
define PREBUILDCMDS
endef
define PRELINKCMDS
endef
define POSTBUILDCMDS
endef

ifeq ($(config),release_win32)
TARGETDIR = ../../bin/release
TARGET = $(TARGETDIR)/premake5.exe
OBJDIR = obj/Win32/Release/Premake5
DEFINES += -DPREMAKE_COMPRESSION -DCURL_STATICLIB -DPREMAKE_CURL -DNDEBUG
ALL_CFLAGS += $(CFLAGS) $(ALL_CPPFLAGS) -flto -O3 -Wall -Wextra
ALL_CXXFLAGS += $(CXXFLAGS) $(ALL_CPPFLAGS) -flto -O3 -Wall -Wextra -fno-stack-protector
LIBS += bin/Win32/Release/lua-lib.lib bin/Win32/Release/zip-lib.lib bin/Win32/Release/zlib-lib.lib bin/Win32/Release/curl-lib.lib -lole32 -lws2_32 -ladvapi32 -lversion
LDDEPS += bin/Win32/Release/lua-lib.lib bin/Win32/Release/zip-lib.lib bin/Win32/Release/zlib-lib.lib bin/Win32/Release/curl-lib.lib
ALL_LDFLAGS += $(LDFLAGS) -flto -s

else ifeq ($(config),debug_win32)
TARGETDIR = ../../bin/debug
TARGET = $(TARGETDIR)/premake5.exe
OBJDIR = obj/Win32/Debug/Premake5
DEFINES += -DPREMAKE_COMPRESSION -DCURL_STATICLIB -DPREMAKE_CURL -D_DEBUG
ALL_CFLAGS += $(CFLAGS) $(ALL_CPPFLAGS) -g -Wall -Wextra
ALL_CXXFLAGS += $(CXXFLAGS) $(ALL_CPPFLAGS) -g -Wall -Wextra
LIBS += bin/Win32/Debug/lua-lib.lib bin/Win32/Debug/zip-lib.lib bin/Win32/Debug/zlib-lib.lib bin/Win32/Debug/curl-lib.lib -lole32 -lws2_32 -ladvapi32 -lversion
LDDEPS += bin/Win32/Debug/lua-lib.lib bin/Win32/Debug/zip-lib.lib bin/Win32/Debug/zlib-lib.lib bin/Win32/Debug/curl-lib.lib
ALL_LDFLAGS += $(LDFLAGS)

endif

# Per File Configurations
# #############################################


# File sets
# #############################################

CUSTOM :=
GENERATED :=
OBJECTS :=

CUSTOM += $(OBJDIR)/resource.res
GENERATED += $(OBJDIR)/buffered_io.o
GENERATED += $(OBJDIR)/criteria_matches.o
GENERATED += $(OBJDIR)/curl_utils.o
GENERATED += $(OBJDIR)/debug_prompt.o
GENERATED += $(OBJDIR)/http_download.o
GENERATED += $(OBJDIR)/http_get.o
GENERATED += $(OBJDIR)/http_post.o
GENERATED += $(OBJDIR)/lua_auxlib.o
GENERATED += $(OBJDIR)/os_chdir.o
GENERATED += $(OBJDIR)/os_chmod.o
GENERATED += $(OBJDIR)/os_comparefiles.o
GENERATED += $(OBJDIR)/os_compile.o
GENERATED += $(OBJDIR)/os_copyfile.o
GENERATED += $(OBJDIR)/os_getWindowsRegistry.o
GENERATED += $(OBJDIR)/os_getcwd.o
GENERATED += $(OBJDIR)/os_getpass.o
GENERATED += $(OBJDIR)/os_getversion.o
GENERATED += $(OBJDIR)/os_host.o
GENERATED += $(OBJDIR)/os_is64bit.o
GENERATED += $(OBJDIR)/os_isdir.o
GENERATED += $(OBJDIR)/os_isfile.o
GENERATED += $(OBJDIR)/os_islink.o
GENERATED += $(OBJDIR)/os_listWindowsRegistry.o
GENERATED += $(OBJDIR)/os_locate.o
GENERATED += $(OBJDIR)/os_match.o
GENERATED += $(OBJDIR)/os_mkdir.o
GENERATED += $(OBJDIR)/os_pathsearch.o
GENERATED += $(OBJDIR)/os_realpath.o
GENERATED += $(OBJDIR)/os_remove.o
GENERATED += $(OBJDIR)/os_rename.o
GENERATED += $(OBJDIR)/os_rmdir.o
GENERATED += $(OBJDIR)/os_stat.o
GENERATED += $(OBJDIR)/os_touchfile.o
GENERATED += $(OBJDIR)/os_uuid.o
GENERATED += $(OBJDIR)/os_writefile_ifnotequal.o
GENERATED += $(OBJDIR)/path_getabsolute.o
GENERATED += $(OBJDIR)/path_getrelative.o
GENERATED += $(OBJDIR)/path_isabsolute.o
GENERATED += $(OBJDIR)/path_join.o
GENERATED += $(OBJDIR)/path_normalize.o
GENERATED += $(OBJDIR)/path_translate.o
GENERATED += $(OBJDIR)/path_wildcards.o
GENERATED += $(OBJDIR)/premake.o
GENERATED += $(OBJDIR)/premake_main.o
GENERATED += $(OBJDIR)/resource.res
GENERATED += $(OBJDIR)/scripts.o
GENERATED += $(OBJDIR)/string_endswith.o
GENERATED += $(OBJDIR)/string_hash.o
GENERATED += $(OBJDIR)/string_sha1.o
GENERATED += $(OBJDIR)/string_startswith.o
GENERATED += $(OBJDIR)/term_textColor.o
GENERATED += $(OBJDIR)/zip_extract.o
OBJECTS += $(OBJDIR)/buffered_io.o
OBJECTS += $(OBJDIR)/criteria_matches.o
OBJECTS += $(OBJDIR)/curl_utils.o
OBJECTS += $(OBJDIR)/debug_prompt.o
OBJECTS += $(OBJDIR)/http_download.o
OBJECTS += $(OBJDIR)/http_get.o
OBJECTS += $(OBJDIR)/http_post.o
OBJECTS += $(OBJDIR)/lua_auxlib.o
OBJECTS += $(OBJDIR)/os_chdir.o
OBJECTS += $(OBJDIR)/os_chmod.o
OBJECTS += $(OBJDIR)/os_comparefiles.o
OBJECTS += $(OBJDIR)/os_compile.o
OBJECTS += $(OBJDIR)/os_copyfile.o
OBJECTS += $(OBJDIR)/os_getWindowsRegistry.o
OBJECTS += $(OBJDIR)/os_getcwd.o
OBJECTS += $(OBJDIR)/os_getpass.o
OBJECTS += $(OBJDIR)/os_getversion.o
OBJECTS += $(OBJDIR)/os_host.o
OBJECTS += $(OBJDIR)/os_is64bit.o
OBJECTS += $(OBJDIR)/os_isdir.o
OBJECTS += $(OBJDIR)/os_isfile.o
OBJECTS += $(OBJDIR)/os_islink.o
OBJECTS += $(OBJDIR)/os_listWindowsRegistry.o
OBJECTS += $(OBJDIR)/os_locate.o
OBJECTS += $(OBJDIR)/os_match.o
OBJECTS += $(OBJDIR)/os_mkdir.o
OBJECTS += $(OBJDIR)/os_pathsearch.o
OBJECTS += $(OBJDIR)/os_realpath.o
OBJECTS += $(OBJDIR)/os_remove.o
OBJECTS += $(OBJDIR)/os_rename.o
OBJECTS += $(OBJDIR)/os_rmdir.o
OBJECTS += $(OBJDIR)/os_stat.o
OBJECTS += $(OBJDIR)/os_touchfile.o
OBJECTS += $(OBJDIR)/os_uuid.o
OBJECTS += $(OBJDIR)/os_writefile_ifnotequal.o
OBJECTS += $(OBJDIR)/path_getabsolute.o
OBJECTS += $(OBJDIR)/path_getrelative.o
OBJECTS += $(OBJDIR)/path_isabsolute.o
OBJECTS += $(OBJDIR)/path_join.o
OBJECTS += $(OBJDIR)/path_normalize.o
OBJECTS += $(OBJDIR)/path_translate.o
OBJECTS += $(OBJDIR)/path_wildcards.o
OBJECTS += $(OBJDIR)/premake.o
OBJECTS += $(OBJDIR)/premake_main.o
OBJECTS += $(OBJDIR)/scripts.o
OBJECTS += $(OBJDIR)/string_endswith.o
OBJECTS += $(OBJDIR)/string_hash.o
OBJECTS += $(OBJDIR)/string_sha1.o
OBJECTS += $(OBJDIR)/string_startswith.o
OBJECTS += $(OBJDIR)/term_textColor.o
OBJECTS += $(OBJDIR)/zip_extract.o

# Rules
# #############################################

all: $(TARGET)
	@:

$(TARGET): $(CUSTOM) $(GENERATED) $(OBJECTS) $(LDDEPS) | $(TARGETDIR)
	$(PRELINKCMDS)
	@echo Linking Premake5
	$(SILENT) $(LINKCMD)
	$(POSTBUILDCMDS)

$(TARGETDIR):
	@echo Creating $(TARGETDIR)
ifeq (posix,$(SHELLTYPE))
	$(SILENT) mkdir -p $(TARGETDIR)
else
	$(SILENT) mkdir $(subst /,\\,$(TARGETDIR))
endif

$(OBJDIR):
	@echo Creating $(OBJDIR)
ifeq (posix,$(SHELLTYPE))
	$(SILENT) mkdir -p $(OBJDIR)
else
	$(SILENT) mkdir $(subst /,\\,$(OBJDIR))
endif

clean:
	@echo Cleaning Premake5
ifeq (posix,$(SHELLTYPE))
	$(SILENT) rm -f  $(TARGET)
	$(SILENT) rm -rf $(GENERATED)
	$(SILENT) rm -rf $(OBJDIR)
else
	$(SILENT) if exist $(subst /,\\,$(TARGET)) del $(subst /,\\,$(TARGET))
	$(SILENT) if exist $(subst /,\\,$(GENERATED)) rmdir /s /q $(subst /,\\,$(GENERATED))
	$(SILENT) if exist $(subst /,\\,$(OBJDIR)) rmdir /s /q $(subst /,\\,$(OBJDIR))
endif

prebuild: | $(OBJDIR)
	$(PREBUILDCMDS)

$(CUSTOM): | prebuild
ifneq (,$(PCH))
$(OBJECTS): $(GCH) | $(PCH_PLACEHOLDER)
$(GCH): $(PCH) | prebuild
	@echo $(notdir $<)
	$(SILENT) $(CC) -x c-header $(ALL_CFLAGS) -o "$@" -MF "$(@:%.gch=%.d)" -c "$<"
$(PCH_PLACEHOLDER): $(GCH) | $(OBJDIR)
ifeq (posix,$(SHELLTYPE))
	$(SILENT) touch "$@"
else
	$(SILENT) echo $null >> "$@"
endif
else
$(OBJECTS): | prebuild
endif


# File Rules
# #############################################

$(OBJDIR)/buffered_io.o: ../../src/host/buffered_io.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/criteria_matches.o: ../../src/host/criteria_matches.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/curl_utils.o: ../../src/host/curl_utils.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/debug_prompt.o: ../../src/host/debug_prompt.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/http_download.o: ../../src/host/http_download.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/http_get.o: ../../src/host/http_get.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/http_post.o: ../../src/host/http_post.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/lua_auxlib.o: ../../src/host/lua_auxlib.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/os_chdir.o: ../../src/host/os_chdir.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/os_chmod.o: ../../src/host/os_chmod.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/os_comparefiles.o: ../../src/host/os_comparefiles.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/os_compile.o: ../../src/host/os_compile.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/os_copyfile.o: ../../src/host/os_copyfile.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/os_getWindowsRegistry.o: ../../src/host/os_getWindowsRegistry.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/os_getcwd.o: ../../src/host/os_getcwd.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/os_getpass.o: ../../src/host/os_getpass.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/os_getversion.o: ../../src/host/os_getversion.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/os_host.o: ../../src/host/os_host.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/os_is64bit.o: ../../src/host/os_is64bit.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/os_isdir.o: ../../src/host/os_isdir.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/os_isfile.o: ../../src/host/os_isfile.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/os_islink.o: ../../src/host/os_islink.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/os_listWindowsRegistry.o: ../../src/host/os_listWindowsRegistry.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/os_locate.o: ../../src/host/os_locate.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/os_match.o: ../../src/host/os_match.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/os_mkdir.o: ../../src/host/os_mkdir.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/os_pathsearch.o: ../../src/host/os_pathsearch.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/os_realpath.o: ../../src/host/os_realpath.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/os_remove.o: ../../src/host/os_remove.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/os_rename.o: ../../src/host/os_rename.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/os_rmdir.o: ../../src/host/os_rmdir.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/os_stat.o: ../../src/host/os_stat.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/os_touchfile.o: ../../src/host/os_touchfile.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/os_uuid.o: ../../src/host/os_uuid.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/os_writefile_ifnotequal.o: ../../src/host/os_writefile_ifnotequal.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/path_getabsolute.o: ../../src/host/path_getabsolute.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/path_getrelative.o: ../../src/host/path_getrelative.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/path_isabsolute.o: ../../src/host/path_isabsolute.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/path_join.o: ../../src/host/path_join.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/path_normalize.o: ../../src/host/path_normalize.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/path_translate.o: ../../src/host/path_translate.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/path_wildcards.o: ../../src/host/path_wildcards.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/premake.o: ../../src/host/premake.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/premake_main.o: ../../src/host/premake_main.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/resource.res: ../../src/host/resource.rc
	@echo $(notdir $<)
	$(SILENT) $(RESCOMP) $< -O coff -o "$@" $(ALL_RESFLAGS)
$(OBJDIR)/string_endswith.o: ../../src/host/string_endswith.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/string_hash.o: ../../src/host/string_hash.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/string_sha1.o: ../../src/host/string_sha1.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/string_startswith.o: ../../src/host/string_startswith.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/term_textColor.o: ../../src/host/term_textColor.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/zip_extract.o: ../../src/host/zip_extract.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"
$(OBJDIR)/scripts.o: ../../src/scripts.c
	@echo $(notdir $<)
	$(SILENT) $(CC) $(ALL_CFLAGS) $(FORCE_INCLUDE) -o "$@" -MF "$(@:%.o=%.d)" -c "$<"

-include $(OBJECTS:%.o=%.d)
ifneq (,$(PCH))
  -include $(PCH_PLACEHOLDER).d
endif