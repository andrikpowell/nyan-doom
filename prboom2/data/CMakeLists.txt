cmake_minimum_required(VERSION 3.8...4.0)
project(nyan-doom-wad
    LANGUAGES C
)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    list(APPEND
        CMAKE_MODULE_PATH
        "${CMAKE_CURRENT_LIST_DIR}/../cmake"
    )
endif()

# PrBoom-Plus WAD creation tool

add_executable(nyan_rdatawad
    rd_main.c
    rd_util.c
    rd_util.h
    rd_output.c
    rd_output.h
    rd_sound.c
    rd_sound.h
    rd_palette.c
    rd_palette.h
    rd_graphic.c
    rd_graphic.h
)

target_compile_features(nyan_rdatawad
    PRIVATE c_std_99
)
set_target_properties(nyan_rdatawad
    PROPERTIES
    OUTPUT_NAME rdatawad
)

include(NyanTargetFeatures)
nyan_target_set_warnings(nyan_rdatawad)
nyan_target_silence_deprecation(nyan_rdatawad)

include(CheckBigEndian)
check_big_endian(RD_IS_BIG_ENDIAN)

target_compile_definitions(nyan_rdatawad
    PRIVATE
    "RD_IS_BIG_ENDIAN=${RD_IS_BIG_ENDIAN}"
)

# PrBoom-Plus internal WAD

set(PALETTE
    palette.rgb
)

set(LUMPS
    lumps/switches.lmp
    lumps/animated.lmp
    lumps/sndcurve.lmp
    lumps/soundfont/sndfont.lmp
    lumps/dsdacr.lmp
    lumps/nyanhud.lmp
    lumps/x11r6rgb.lmp
)

set(COLORMAPS
    lumps/watermap.lmp
)

set(PRBOOM_LUMPS
    lumps/sinetabl.lmp
    lumps/tangtabl.lmp
    lumps/tantoang.lmp
    lumps/gammatbl.lmp
    lumps/chexdeh.lmp
    lumps/bfgbex.lmp
    lumps/nervebex.lmp
    lumps/glshadow.lmp
    lumps/gls_main.lmp
    lumps/gls_fuzz.lmp
    lumps/gls_v.lmp
)

set(OPENGL_GRAPHICS
    graphics/opengl/m_ammo.png
    graphics/opengl/m_armour.png
    graphics/opengl/m_arrow.png
    graphics/opengl/m_health.png
    graphics/opengl/m_key.png
    graphics/opengl/m_normal.png
    graphics/opengl/m_shadow.png
    graphics/opengl/m_power.png
    graphics/opengl/m_weap.png
    graphics/opengl/m_mark.png
    graphics/opengl/h_ammo.png
    graphics/opengl/h_weap.png
    graphics/opengl/h_weap2.png
    graphics/opengl/h_mana.png
    graphics/opengl/h_puzzle.png
)

set(SOUNDS
    sounds/dsdgsit.wav
    sounds/dsdgatk.wav
    sounds/dsdgact.wav
    sounds/dsdgdth.wav
    sounds/dsdgpain.wav
    sounds/dssecret.wav
    sounds/dsgibdth.wav
    sounds/dsnyanut.wav
)

set (PPM_GRAPHICS
    # dsda font - raven looks weird with PNG
    graphics/dig033.ppm
    graphics/dig034.ppm
    graphics/dig035.ppm
    graphics/dig036.ppm
    graphics/dig037.ppm
    graphics/dig038.ppm
    graphics/dig039.ppm
    graphics/dig040.ppm
    graphics/dig041.ppm
    graphics/dig042.ppm
    graphics/dig043.ppm
    graphics/dig044.ppm
    graphics/dig045.ppm
    graphics/dig046.ppm
    graphics/dig047.ppm
    graphics/dig048.ppm
    graphics/dig049.ppm
    graphics/dig050.ppm
    graphics/dig051.ppm
    graphics/dig052.ppm
    graphics/dig053.ppm
    graphics/dig054.ppm
    graphics/dig055.ppm
    graphics/dig056.ppm
    graphics/dig057.ppm
    graphics/dig058.ppm
    graphics/dig059.ppm
    graphics/dig060.ppm
    graphics/dig061.ppm
    graphics/dig062.ppm
    graphics/dig063.ppm
    graphics/dig064.ppm
    graphics/dig065.ppm
    graphics/dig066.ppm
    graphics/dig067.ppm
    graphics/dig068.ppm
    graphics/dig069.ppm
    graphics/dig070.ppm
    graphics/dig071.ppm
    graphics/dig072.ppm
    graphics/dig073.ppm
    graphics/dig074.ppm
    graphics/dig075.ppm
    graphics/dig076.ppm
    graphics/dig077.ppm
    graphics/dig078.ppm
    graphics/dig079.ppm
    graphics/dig080.ppm
    graphics/dig081.ppm
    graphics/dig082.ppm
    graphics/dig083.ppm
    graphics/dig084.ppm
    graphics/dig085.ppm
    graphics/dig086.ppm
    graphics/dig087.ppm
    graphics/dig088.ppm
    graphics/dig089.ppm
    graphics/dig090.ppm
    graphics/dig091.ppm
    graphics/dig092.ppm
    graphics/dig093.ppm
    graphics/dig094.ppm
    graphics/dig095.ppm
    graphics/dig096.ppm
    graphics/dig097.ppm
    graphics/dig098.ppm
    graphics/dig099.ppm
    graphics/dig100.ppm
    graphics/dig101.ppm
    graphics/dig102.ppm
    graphics/dig103.ppm
    graphics/dig104.ppm
    graphics/dig105.ppm
    graphics/dig106.ppm
    graphics/dig107.ppm
    graphics/dig108.ppm
    graphics/dig109.ppm
    graphics/dig110.ppm
    graphics/dig111.ppm
    graphics/dig112.ppm
    graphics/dig113.ppm
    graphics/dig114.ppm
    graphics/dig115.ppm
    graphics/dig116.ppm
    graphics/dig117.ppm
    graphics/dig118.ppm
    graphics/dig119.ppm
    graphics/dig120.ppm
    graphics/dig121.ppm
    graphics/dig122.ppm
    graphics/dig123.ppm
    graphics/dig124.ppm
    graphics/dig125.ppm
    graphics/dig126.ppm

    # must be ppm to match palette
    graphics/m_colors.ppm
)

set(GRAPHICS
    # raven font
    graphics/fonta06.png
    graphics/fonta28.png
    graphics/fonta30.png

    # doom font char
    graphics/stcfn096.png

    # prboom graphics
    graphics/boxul.png
    graphics/boxuc.png
    graphics/boxur.png
    graphics/boxcl.png
    graphics/boxcc.png
    graphics/boxcr.png
    graphics/boxll.png
    graphics/boxlc.png
    graphics/boxlr.png
    graphics/m_butt1.png
    graphics/m_butt2.png
    graphics/m_palno.png
    graphics/m_palsel.png
    graphics/m_vbox.png

    # keys
    graphics/stkeys6.png
    graphics/stkeys7.png
    graphics/stkeys8.png

    # crosshairs
    graphics/cross1.png
    graphics/cross2.png
    graphics/cross3.png
    graphics/cross4.png
    graphics/cross5.png
    graphics/cross6.png
    graphics/cross7.png

    # Weapon carousel graphics from Woof, drawn by Julia Nechaevskaya
    graphics/smbfgg0.png
    graphics/smbfgg1.png
    graphics/smcsaw0.png
    graphics/smcsaw1.png
    graphics/smfist0.png
    graphics/smfist1.png
    graphics/smlaun0.png
    graphics/smlaun1.png
    graphics/smmgun0.png
    graphics/smmgun1.png
    graphics/smpisg0.png
    graphics/smpisg1.png
    graphics/smplas0.png
    graphics/smplas1.png
    graphics/smsgn20.png
    graphics/smsgn21.png
    graphics/smshot0.png
    graphics/smshot1.png
)

 # must be ppm to match palette
set(COLOR_PICKER
    graphics/m_colors.ppm
)

set(NYAN_GRAPHICS
    # nyan
    graphics/nyanlogo.png

    # boom bar
    graphics/nyanstb1.png
    graphics/nyanstb2.png
    graphics/nyanstb3.png

    # map icons
    graphics/amkills.png
    graphics/amitem.png
    graphics/amsecr.png
    graphics/amkills2.png
    graphics/amitem2.png
    graphics/amsecr2.png
    graphics/amkills3.png
    graphics/amitem3.png
    graphics/amsecr3.png

    # hud graphics
    graphics/stfpstr0.png
    graphics/stfpstr1.png
    graphics/stfarms0.png
    graphics/stfarms1.png
    graphics/stfarms2.png
    graphics/stfarm20.png
    graphics/stfarm21.png
    graphics/stfarm22.png
    graphics/chxpstr0.png
    graphics/chxpstr1.png
    graphics/chxarms0.png
    graphics/chxarms1.png
    graphics/chxarms2.png

    # status widget
    graphics/stfpkill.png
    graphics/stfpitem.png
    graphics/stfpsecr.png
    graphics/stfrkil2.png
    graphics/stfritm2.png
    graphics/stfrscr2.png
    graphics/stfrkil3.png
    graphics/stfritm3.png
    graphics/stfrscr3.png
    graphics/stfrtome.png
    graphics/stfrmor1.png
    graphics/stfrmor2.png
    graphics/stfrsped.png
    graphics/stfrmaul.png
    graphics/stfpinv.png
    graphics/stfpins.png
    graphics/stfpvis.png
    graphics/stfpmap.png
    graphics/stfpsuit.png
    graphics/stfpbpak.png
    graphics/stfppstr.png
    graphics/stfparm1.png
    graphics/stfparm2.png
    graphics/stfpfly.png
)

set(FLATS
    flats/-n0_tex-.ppm
)

set(SPRITES
    sprites/tnt1a0.png

    sprites/dogsa1.png
    sprites/dogsa2.png
    sprites/dogsa3.png
    sprites/dogsa4.png
    sprites/dogsa5.png
    sprites/dogsa6.png
    sprites/dogsa7.png
    sprites/dogsa8.png
    sprites/dogsb1.png
    sprites/dogsb2.png
    sprites/dogsb3.png
    sprites/dogsb4.png
    sprites/dogsb5.png
    sprites/dogsb6.png
    sprites/dogsb7.png
    sprites/dogsb8.png
    sprites/dogsc1.png
    sprites/dogsc2.png
    sprites/dogsc3.png
    sprites/dogsc4.png
    sprites/dogsc5.png
    sprites/dogsc6.png
    sprites/dogsc7.png
    sprites/dogsc8.png
    sprites/dogsd1.png
    sprites/dogsd2.png
    sprites/dogsd3.png
    sprites/dogsd4.png
    sprites/dogsd5.png
    sprites/dogsd6.png
    sprites/dogsd7.png
    sprites/dogsd8.png
    sprites/dogse1.png
    sprites/dogse2.png
    sprites/dogse3.png
    sprites/dogse4.png
    sprites/dogse5.png
    sprites/dogse6.png
    sprites/dogse7.png
    sprites/dogse8.png
    sprites/dogsf1.png
    sprites/dogsf2.png
    sprites/dogsf3.png
    sprites/dogsf4.png
    sprites/dogsf5.png
    sprites/dogsf6.png
    sprites/dogsf7.png
    sprites/dogsf8.png
    sprites/dogsg1.png
    sprites/dogsg2.png
    sprites/dogsg3.png
    sprites/dogsg4.png
    sprites/dogsg5.png
    sprites/dogsg6.png
    sprites/dogsg7.png
    sprites/dogsg8.png
    sprites/dogsh1.png
    sprites/dogsh2.png
    sprites/dogsh3.png
    sprites/dogsh4.png
    sprites/dogsh5.png
    sprites/dogsh6.png
    sprites/dogsh7.png
    sprites/dogsh8.png
    sprites/dogsi0.png
    sprites/dogsj0.png
    sprites/dogsk0.png
    sprites/dogsl0.png
    sprites/dogsm0.png
    sprites/dogsn0.png

    sprites/pls1a0.png
    sprites/pls1b0.png
    sprites/pls1c0.png
    sprites/pls1d0.png
    sprites/pls1e0.png
    sprites/pls1f0.png
    sprites/pls1g0.png
    sprites/pls2a0.png
    sprites/pls2b0.png
    sprites/pls2c0.png
    sprites/pls2d0.png
    sprites/pls2e0.png
)

set(WAD_SRC
    ${PALETTE}
    ${LUMPS}
    ${COLORMAPS}
    ${PRBOOM_LUMPS}
    ${OPENGL_GRAPHICS}
    ${SOUNDS}
    ${PPM_GRAPHICS}
    ${GRAPHICS}
    ${NYAN_GRAPHICS}
    ${FLATS}
    ${SPRITES}
)

set(WAD_CMDLINE
    -palette ${PALETTE}
    -lumps ${LUMPS}
    -marker C_START
        -lumps ${COLORMAPS}
    -marker C_END
    -marker B_START
        -lumps ${PRBOOM_LUMPS}
        -pngs ${OPENGL_GRAPHICS}
    -marker B_END
    -sounds ${SOUNDS}
    -graphics ${PPM_GRAPHICS}
    -pngs ${GRAPHICS}
    -pngs ${NYAN_GRAPHICS}
    -marker FF_START
        -flats ${FLATS}
    -marker FF_END
    -marker SS_START
        -pngs ${SPRITES}
    -marker SS_END
)

add_custom_command(
    OUTPUT ${WAD_DATA_PATH}
    COMMAND $<TARGET_FILE:nyan_rdatawad> -I "${CMAKE_CURRENT_SOURCE_DIR}" ${WAD_CMDLINE} -o "${WAD_DATA_PATH}"
    DEPENDS nyan_rdatawad ${WAD_SRC}
    VERBATIM
)

add_custom_target(nyan-doom-wad 
    ALL
    DEPENDS ${WAD_DATA_PATH}
)
