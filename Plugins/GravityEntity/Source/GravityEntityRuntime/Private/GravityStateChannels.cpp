#include "GravityStateChannels.h"

void UGravityStateChannels::SetChannel(FName ChannelName, float Value)
{
	if      (ChannelName == TEXT("Tension"))        { Tension        = Value; }
	else if (ChannelName == TEXT("Charge"))         { Charge         = Value; }
	else if (ChannelName == TEXT("Speed"))          { Speed          = Value; }
	else if (ChannelName == TEXT("ResonancePhase")) { ResonancePhase = Value; }
	else if (ChannelName == TEXT("Instability"))    { Instability    = Value; }
	else if (ChannelName == TEXT("Attention"))      { Attention      = Value; }
	else if (ChannelName == TEXT("BreathPhase"))    { BreathPhase    = Value; }
	else if (ChannelName == TEXT("BreathAmplitude")){ BreathAmplitude= Value; }
	else                                            { ExtraChannels.Add(ChannelName, Value); }
}

float UGravityStateChannels::GetChannel(FName ChannelName) const
{
	if      (ChannelName == TEXT("Tension"))        { return Tension;         }
	else if (ChannelName == TEXT("Charge"))         { return Charge;          }
	else if (ChannelName == TEXT("Speed"))          { return Speed;           }
	else if (ChannelName == TEXT("ResonancePhase")) { return ResonancePhase;  }
	else if (ChannelName == TEXT("Instability"))    { return Instability;     }
	else if (ChannelName == TEXT("Attention"))      { return Attention;       }
	else if (ChannelName == TEXT("BreathPhase"))    { return BreathPhase;     }
	else if (ChannelName == TEXT("BreathAmplitude")){ return BreathAmplitude; }
	else if (const float* Extra = ExtraChannels.Find(ChannelName)) { return *Extra; }
	return 0.f;
}

void UGravityStateChannels::Reset()
{
	Tension = Charge = Speed = ResonancePhase = Instability = Attention = BreathPhase = BreathAmplitude = 0.f;
	ExtraChannels.Reset();
}
