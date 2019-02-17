cmake_minimum_required(VERSION 2.6)
project(FRPI)
add_executable(
        frpi
        Multiprotocol.c
        CC2500_SPI.c
        FrSkyD_cc2500.c
        FrSkyV_cc2500.c
        FrSkyX_cc2500.c
        Common.c
)

target_link_libraries(frpi wiringPi)

add_definitions(-DATER -DCC2500_INSTALLED -DFRSKYD_CC2500_INO -DFRSKYX_CC2500_INO -DFRSKYV_CC2500_INO)
