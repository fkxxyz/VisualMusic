TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
    AudioDecoder/AudioFileDecoder.cpp \
    AudioDecoder/BaseAudioDecoder.cpp \
    AudioDecoder/Mp3Decoder.cpp \
    AudioDecoder/WavDecoder.cpp \
    WavePlayer/WinMMwaveOut.cpp \
    gui/wxVisualMusicApp.cpp \
    gui/wxVisualMusicFrame.cpp

HEADERS += \
    AudioDecoder/AudioFileDecoder.h \
    AudioDecoder/BaseAudioDecoder.h \
    AudioDecoder/BasePipe.h \
    AudioDecoder/BasePipe.inl \
    AudioDecoder/Mp3Decoder.h \
    AudioDecoder/PCMScaler.h \
    AudioDecoder/PCMScaler.inl \
    AudioDecoder/WavDecoder.h \
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
    SpectrumAnalyser/SpectrumAnalyser.h \
    SpectrumAnalyser/SpectrumAnalyser.inl \
    DrawTemplate/colordef.h \
    DrawTemplate/DrawTemplate.h \
    DrawSpectrum/DrawSpectrum.h \
    gui/wxVisualMusicApp.h \
    gui/wxVisualMusicFrame.h \
    thread/thread.h \
    thread/threadtypes.h \
    thread/thread_common.inl \
    thread/thread_windows.inl \
    thread/thread_linux.inl \
    constants.h \
    VisualMusicController.h \
    stdafx.h

INCLUDEPATH += \
    /usr/lib/wx/include/gtk2-unicode-3.0 \
    /usr/include/wx-3.0

QMAKE_CXXFLAGS += -D_FILE_OFFSET_BITS=64 -DWXUSINGDLL -D__WXGTK__ -pthread

LIBS += \
    `pkg-config --libs alsa mad` `wx-config --libs`

DISTFILES +=
