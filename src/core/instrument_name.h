#pragma once

#include "type_definitions.h"
#include <string.h>

namespace DangoFM
{


  enum InstrumentName : uint8 {
    NoInstrument = 0,
    AcousticPiano = 1,
    BrightPiano,
    ElectricPiano,
    HonkyPiano,
    RhodesPiano,
    ChorusPiano,
    Harpsichord,
    Clavinet,
    Celesta,
    Glockenspiel,

    MusicBox = 11,
    Vibraphone,
    Marimba,
    Xylophone,
    TubularBells,
    Dulcimer,
    HammondOrgan,
    PercussiveOrgan,
    RockOrgan,
    ChurchOrgan,

    ReedOrgan = 21,
    Accordion,
    Harmonica,
    TangoAccordion,
    AcousticNylonGuitar,
    AcousticSteelGuitar,
    ElectricJazzGuitar,
    ElectricCleanGuitar,
    ElectricMutedGuitar,
    OverdrivenGuitar,

    DistortionGuitar = 31,
    GuitarHarmonics,
    AcousticBass,
    ElectricFingeredBass,
    ElectricPickedBass,
    FretlessBass,
    SlapBass1,
    SlapBass2,
    SynthBass1,
    SynthBass2,

    Violin = 41,
    Viola,
    Cello,
    Contrabass,
    TremoloStrings,
    PizzicatoStrings,
    OrchestralHarp,
    Timpani,
    StringEnsemble1,

    StringEnsemble2,
    SynthStrings1,
    SynthStrings2,
    ChoirAahs,
    VoiceOohs,
    SynthVoice,
    OrchestraHit,
    Trumpet,
    Trombone,
    Tuba,
    MutedTrumpet,

    FrenchHorn,
    BrassSection,
    SynthBrass1,
    SynthBrass2,
    SopranoSax,
    AltoSax,
    TenorSax,
    BaritoneSax,
    Oboe,

    EnglishHorn,
    Bassoon,
    Clarinet,
    Piccolo,
    Flute,
    Recorder,
    PanFlute,
    BlownBottle,
    Shakuhachi,
    Whistle,
    Ocarina,

    LeadSquareWave,
    LeadSawWave,
    LeadCalliope,
    LeadChiffer,
    LeadCharang,
    LeadVoice,
    LeadFifths,
    LeadBass,
    PadNewAge,
    PadWarm,

    PadPolysynth,
    PadChoirSpaceVoice,
    PadBowedGlass,
    PadMetallic,
    PadHalo,
    PadSweep,
    FxRain,
    FxSoundtrack,
    FxCrystal,
    FxAthmosphere,

    FxBrightness,
    FxGoblins,
    FxEchoes,
    FxScifi,
    Sitar,
    Banjo,
    Shamisen,
    Koto,
    Kalimba,
    BagPipe,

    Fiddle,
    Shanai,
    TinkleBell,
    Agogo,
    SteelDrums,
    Woodblock,
    TaikoDrum,
    MelodicTom,
    SynthDrum,
    ReverseCymbal,

    GuitarFretNoise,
    BreathNoise,
    Seashore,
    BirdTweet,
    TelephoneRing,
    Helicopter,
    Applause,
    Gunshot
  };

  class InstrumentNames
  {
  public:
    static std::string InstrumentNameStrings[128];
    static std::string get_instrument_name(uint8 index);
    static std::string get_instrument_name(InstrumentName inst);
    static const uint8 instrument_amount = 128;
  };

}
