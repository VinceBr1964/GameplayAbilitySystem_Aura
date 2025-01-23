// Copyright Vince Bracken


#include "UI/ViewModel/MVVM_LoadSlot.h"

void UMVVM_LoadSlot::InitializeSlot()
{
	//Todo Check slot status based on loaded data
	SetWidgetSwitcherIndex.Broadcast(2);
}
