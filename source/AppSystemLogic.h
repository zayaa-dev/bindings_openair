/* Copyright (C) 2005-2023, UNIGINE. All rights reserved.
 *
 * This file is a part of the UNIGINE 2 SDK.
 *
 * Your use and / or redistribution of this software in source and / or
 * binary form, with or without modification, is subject to: (i) your
 * ongoing acceptance of and compliance with the terms and conditions of
 * the UNIGINE License Agreement; and (ii) your inclusion of this notice
 * in any version of this software that you use or redistribute.
 * A copy of the UNIGINE License Agreement is available by contacting
 * UNIGINE. at http://unigine.com/
 */


#ifndef __APP_SYSTEM_LOGIC_H__
#define __APP_SYSTEM_LOGIC_H__

//#include "Bindings.h"
#include "BonusBindings.h"

#include "UndoStack.h"

#include <UnigineDecals.h>
#include <UnigineLog.h>
#include <UnigineLogic.h>

struct NumberUi
{
	Unigine::WidgetEditLinePtr edit_line;
	Unigine::WidgetSliderPtr slider;
};

class AppSystemLogic : public Unigine::SystemLogic
{
public:
	AppSystemLogic();
	~AppSystemLogic() override;

	int init() override;

	int update() override;
	int postUpdate() override;

	int shutdown() override;
private:
	Unigine::DecalOrthoPtr decal_;

	NumberUi width_ui_;
	NumberUi height_ui_;

	UndoStack undo_stack_;
	binds::Binder<Unigine::DecalOrtho> binder_;
};

#endif // __APP_SYSTEM_LOGIC_H__
