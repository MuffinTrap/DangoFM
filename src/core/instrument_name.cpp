#include "instrument_name.h"

std::string DangoFM::InstrumentNames::InstrumentNameStrings[128] = {
  "AcousticPiano",
  "BrightPiano",
  "ElectricPiano",
  "HonkyPiano",
  "RhodesPiano",
  "ChorusPiano",
  "Harpsichord",
  "Clavinet",
  "Celesta",
  "Glockenspiel",

  "MusicBox",
  "Vibraphone",
  "Marimba",
  "Xylophone",
  "TubularBells",
  "Dulcimer",
  "HammondOrgan",
  "PercussiveOrgan",
  "RockOrgan",
  "ChurchOrgan",

  "ReedOrgan",
  "Accordion",
  "Harmonica",
  "TangoAccordion",
  "AcousticNylonGuitar",
  "AcousticSteelGuitar",
  "ElectricJazzGuitar",
  "ElectricCleanGuitar",
  "ElectricMutedGuitar",
  "OverdrivenGuitar",

  "DistortionGuitar",
  "GuitarHarmonics",
  "AcousticBass",
  "ElectricFingeredBass",
  "ElectricPickedBass",
  "FretlessBass",
  "SlapBass1",
  "SlapBass2",
  "SynthBass1",
  "SynthBass2",

  "Violin",
  "Viola",
  "Cello",
  "Contrabass",
  "TremoloStrings",
  "PizzicatoStrings",
  "OrchestralHarp",
  "Timpani",
  "StringEnsemble1",

  "StringEnsemble2",
  "SynthStrings1",
  "SynthStrings2",
  "ChoirAahs",
  "VoiceOohs",
  "SynthVoice",
  "OrchestraHit",
  "Trumpet",
  "Trombone",
  "Tuba",
  "MutedTrumpet",

  "FrenchHorn",
  "BrassSection",
  "SynthBrass1",
  "SynthBrass2",
  "SopranoSax",
  "AltoSax",
  "TenorSax",
  "BaritoneSax",
  "Oboe",

  "EnglishHorn",
  "Bassoon",
  "Clarinet",
  "Piccolo",
  "Flute",
  "Recorder",
  "PanFlute",
  "BlownBottle",
  "Shakuhachi",
  "Whistle",
  "Ocarina",

  "LeadSquareWave",
  "LeadSawWave",
  "LeadCalliope",
  "LeadChiffer",
  "LeadCharang",
  "LeadVoice",
  "LeadFifths",
  "LeadBass",
  "PadNewAge",
  "PadWarm",

  "PadPolysynth",
  "PadChoirSpaceVoice",
  "PadBowedGlass",
  "PadMetallic",
  "PadHalo",
  "PadSweep",
  "FxRain",
  "FxSoundtrack",
  "FxCrystal",
  "FxAthmosphere",

  "FxBrightness",
  "FxGoblins",
  "FxEchoes",
  "FxScifi",
  "Sitar",
  "Banjo",
  "Shamisen",
  "Koto",
  "Kalimba",
  "BagPipe",

  "Fiddle",
  "Shanai",
  "TinkleBell",
  "Agogo",
  "SteelDrums",
  "Woodblock",
  "TaikoDrum",
  "MelodicTom",
  "SynthDrum",
  "ReverseCymbal",

  "GuitarFretNoise",
  "BreathNoise",
  "Seashore",
  "BirdTweet",
  "TelephoneRing",
  "Helicopter",
  "Applause",
  "Gunshot"
};

std::string DangoFM::InstrumentNames::get_instrument_name(uint8 index)
{
  if (index >= 0 && index <= instrument_amount) {
    return InstrumentNameStrings[index];
  }
  else {
    return "Instrument??";
  }
}

std::string DangoFM::InstrumentNames::get_instrument_name(InstrumentName inst) {
  if (inst >= 1 && inst <= instrument_amount) {
    return InstrumentNameStrings[inst - 1];
  }
  else {
    return "Instrument??";
  }
}
