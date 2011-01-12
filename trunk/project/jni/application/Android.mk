LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := application

APP_SUBDIRS := $(patsubst $(LOCAL_PATH)/%, %, $(shell find $(LOCAL_PATH)/src/ -type d))
ifneq ($(APPLICATION_SUBDIRS_BUILD),)
APP_SUBDIRS := $(APPLICATION_SUBDIRS_BUILD)
endif

LOCAL_CFLAGS :=  

ifeq ($(CRYSTAX_TOOLCHAIN),)
# Paths should be on newline so launchConfigure.sh will work properly
LOCAL_CFLAGS += \
				-I$(LOCAL_PATH)/../stlport/stlport
endif

# Paths should be on newline so launchConfigure.sh will work properly
LOCAL_CFLAGS += \
				$(foreach D, $(APP_SUBDIRS), -I$(LOCAL_PATH)/$(D)) \
				-I$(LOCAL_PATH)/../sdl/include \
				-I$(LOCAL_PATH)/../sdl_mixer \
				-I$(LOCAL_PATH)/../sdl_image \
				-I$(LOCAL_PATH)/../sdl_ttf \
				-I$(LOCAL_PATH)/../sdl_net \
				-I$(LOCAL_PATH)/../sdl_blitpool \
				-I$(LOCAL_PATH)/../sdl_gfx \
				-I$(LOCAL_PATH)/../png \
				-I$(LOCAL_PATH)/../jpeg \
				-I$(LOCAL_PATH)/../intl \
				-I$(LOCAL_PATH)/../freetype/include \
				-I$(LOCAL_PATH)/..

LOCAL_CFLAGS += $(APPLICATION_ADDITIONAL_CFLAGS) -DNORMALUNIX -DLINUX -DHAVE_CONFIG_H -DHAVE_MIXER

#Change C++ file extension as appropriate
LOCAL_CPP_EXTENSION := .cpp

#LOCAL_SRC_FILES := $(foreach F, $(APP_SUBDIRS), $(addprefix $(F)/,$(notdir $(wildcard $(LOCAL_PATH)/$(F)/*.cpp))))
# Uncomment to also add C sources
#LOCAL_SRC_FILES += $(foreach F, $(APP_SUBDIRS), $(addprefix $(F)/,$(notdir $(wildcard $(LOCAL_PATH)/$(F)/*.c))))
COMMON_SRC = am_map.c\
am_map.h\
config.h\
d_client.c\
d_deh.c\
d_deh.h\
d_englsh.h\
d_event.h\
d_items.c\
d_items.h\
d_main.c\
d_main.h\
d_net.h\
d_player.h\
d_think.h\
d_ticcmd.h\
doomdata.h\
doomdef.c\
doomdef.h\
doomstat.c\
doomstat.h\
doomtype.h\
drawasm.h\
dstrings.c\
dstrings.h\
f_finale.c\
f_finale.h\
f_wipe.c\
f_wipe.h\
g_game.c\
g_game.h\
hu_lib.c\
hu_lib.h\
hu_stuff.c\
hu_stuff.h\
SDL/i_joy.c\
i_joy.h\
SDL/i_main.c\
i_main.h\
i_network.h\
SDL/i_sound.c\
i_sound.h\
SDL/i_sshot.c\
SDL/i_system.c\
i_system.h\
SDL/i_network.c\
SDL/i_video.c\
i_video.h\
info.c\
info.h\
lprintf.c\
lprintf.h\
m_argv.c\
m_argv.h\
m_bbox.c\
m_bbox.h\
m_cheat.c\
m_cheat.h\
m_fixed.h\
m_menu.c\
m_menu.h\
m_misc.c\
m_misc.h\
m_random.c\
m_random.h\
m_swap.h\
md5.c\
md5.h\
Mmus2mid.c\
Mmus2mid.h\
p_ceilng.c\
p_checksum.c\
p_checksum.h\
p_doors.c\
p_enemy.c\
p_enemy.h\
p_floor.c\
p_genlin.c\
p_inter.c\
p_inter.h\
p_lights.c\
p_map.c\
p_map.h\
p_maputl.c\
p_maputl.h\
p_mobj.c\
p_mobj.h\
p_plats.c\
p_pspr.c\
p_pspr.h\
p_saveg.c\
p_saveg.h\
p_setup.c\
p_setup.h\
p_sight.c\
p_spec.c\
p_spec.h\
p_switch.c\
p_telept.c\
p_tick.c\
p_tick.h\
p_user.c\
p_user.h\
protocol.h\
r_bsp.c\
r_bsp.h\
r_data.c\
r_data.h\
r_defs.h\
r_demo.c\
r_demo.h\
r_draw.c\
r_draw.h\
r_drawcolumn.inl\
r_drawspan.inl\
r_filter.c\
r_filter.h\
r_fps.c\
r_fps.h\
r_main.c\
r_main.h\
r_patch.c\
r_patch.h\
r_plane.c\
r_plane.h\
r_segs.c\
r_segs.h\
r_sky.c\
r_sky.h\
r_state.h\
r_things.c\
r_things.h\
s_sound.c\
s_sound.h\
sounds.c\
sounds.h\
st_lib.c\
st_lib.h\
st_stuff.c\
st_stuff.h\
tables.c\
tables.h\
v_video.c\
v_video.h\
version.c\
version.h\
w_mmap.c\
w_wad.c\
w_wad.h\
wi_stuff.c\
wi_stuff.h\
z_bmalloc.c\
z_bmalloc.h\
z_zone.c\
z_zone.h\





LOCAL_SRC_FILES =  $(foreach F, $(COMMON_SRC), $(addprefix src/src/,$(F))) 
ifneq ($(APPLICATION_CUSTOM_BUILD_SCRIPT),)
LOCAL_SRC_FILES := dummy.c
endif

LOCAL_SHARED_LIBRARIES := sdl $(COMPILED_LIBRARIES)  sdl_mixer

LOCAL_STATIC_LIBRARIES := stlport doomsdl

LOCAL_LDLIBS := -lGLESv1_CM -ldl -llog -lz -lm

LOCAL_LDFLAGS := -Lobj/local/armeabi

LOCAL_LDFLAGS += $(APPLICATION_ADDITIONAL_LDFLAGS)


LIBS_WITH_LONG_SYMBOLS := $(strip $(shell \
	for f in $(LOCAL_PATH)/../../libs/armeabi/*.so ; do \
		if echo $$f | grep "libapplication[.]so" > /dev/null ; then \
			continue ; \
		fi ; \
		if [ -e "$$f" ] ; then \
			if nm -g $$f | cut -c 12- | egrep '.{128}' > /dev/null ; then \
				echo $$f | grep -o 'lib[^/]*[.]so' ; \
			fi ; \
		fi ; \
	done \
) )

ifneq "$(LIBS_WITH_LONG_SYMBOLS)" ""
$(foreach F, $(LIBS_WITH_LONG_SYMBOLS), \
$(info Library $(F): abusing symbol names are: \
$(shell nm -g $(LOCAL_PATH)/../../libs/armeabi/$(F) | cut -c 12- | egrep '.{128}' ) ) \
$(info Library $(F) contains symbol names longer than 128 bytes, \
YOUR CODE WILL DEADLOCK WITHOUT ANY WARNING when you'll access such function - \
please make this library static to avoid problems. ) )
$(error Detected libraries with too long symbol names. Remove all files under project/libs/armeabi, make these libs static, and recompile)
endif

include $(BUILD_SHARED_LIBRARY)

ifneq ($(APPLICATION_CUSTOM_BUILD_SCRIPT),)

$(info LOCAL_PATH $(LOCAL_PATH) )
$(info $(LOCAL_PATH)/src/libapplication.so )
$(info $(realpath $(LOCAL_PATH)/../../obj/local/armeabi/libapplication.so) )

LOCAL_PATH_SDL_APPLICATION := $(LOCAL_PATH)

$(LOCAL_PATH)/src/libapplication.so: $(LOCAL_PATH)/src/AndroidBuild.sh $(LOCAL_PATH)/src/AndroidAppSettings.cfg
	cd $(LOCAL_PATH_SDL_APPLICATION)/src && ./AndroidBuild.sh

# $(realpath $(LOCAL_PATH)/../../libs/armeabi/libapplication.so) \

$(realpath $(LOCAL_PATH)/../../obj/local/armeabi/libapplication.so): $(LOCAL_PATH)/src/libapplication.so OVERRIDE_CUSTOM_LIB
	cp -f $< $@
#	$(patsubst %-gcc,%-strip,$(TARGET_CC)) -g $@

.PHONY: OVERRIDE_CUSTOM_LIB

OVERRIDE_CUSTOM_LIB:

endif
