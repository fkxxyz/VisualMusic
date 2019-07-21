TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    AudioFileDecoder.cpp \
    AudioRawDataPipe.cpp \
    BaseAudioDecoder.cpp \
    Mp3Decoder.cpp \
    Mp3RawDataPipe.cpp \
    PCMPipe.cpp \
    WavePlayer/WinMMwaveOut.cpp \
    spDraw2D.cpp \
    spDrawBar.cpp \
    spDrawBase.cpp \
    SpectrumAnalyser.cpp \
    SpectrumDataPipe.cpp \
    WavDecoder.cpp \
    WavRawDataPipe.cpp \
    wxVisualMusicApp.cpp \
    wxVisualMusicFrame.cpp

HEADERS += \
    AudioFileDecoder.h \
    AudioRawDataPipe.h \
    BaseAudioDecoder.h \
    BasePipe.h \
    BasePipe.inl \
    Mp3Decoder.h \
    Mp3RawDataPipe.h \
    PCMPipe.h \
    WavePlayer/WavePlayer.h \
    WavePlayer/WavePlayer_linux.inl \
    WavePlayer/WavePlayer_windows.inl \
    WavePlayer/WinCritialSection.h \
    WavePlayer/WinEvent.h \
    WavePlayer/WinMMWaveOut.h \
    WavePlayer/WinMMWaveOut.inl \
    WavePlayer/WinObject.h \
    WavePlayer/WinPCMPlayer.h \
    WavePlayer/WinPCMPlayer.inl \
    constants.h \
    spDraw2D.h \
    spDrawBar.h \
    spDrawBase.h \
    SpectrumAnalyser.h \
    SpectrumDataPipe.h \
    WavDecoder.h \
    WavRawDataPipe.h \
    stdafx.h \
    wxVisualMusicApp.h \
    wxVisualMusicFrame.h

INCLUDEPATH += \
    /usr/lib/wx/include/gtk2-unicode-3.0 \
    /usr/include/wx-3.0

QMAKE_CXXFLAGS += -D_FILE_OFFSET_BITS=64 -DWXUSINGDLL -D__WXGTK__ -pthread

LIBS += \
    `pkg-config --libs alsa mad` `wx-config --libs`

DISTFILES +=
