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


#include "AppSystemLogic.h"
#include <UnigineWorld.h>

using namespace Unigine;

// System logic, it exists during the application life cycle.
// These methods are called right after corresponding system script's (UnigineScript) methods.

// Todo attach
// Todo update
// Todo detach

bool binds::compare(float l, float r)
{
	return Math::compare(l, r);
}


NumberUi create_number_ui(const char *name, const WidgetGridBoxPtr &grid)
{
	NumberUi ui;

	GuiPtr gui = grid->getGui();

	auto h_box = WidgetHBox::create(gui);
	h_box->setSpace(5, 0);

	auto label = WidgetLabel::create(gui, name);

	ui.edit_line = WidgetEditLine::create(gui);

	ui.slider = WidgetSlider::create(gui);
	ui.slider->setWidth(165);

	h_box->addChild(ui.edit_line);
	h_box->addChild(ui.slider, Gui::ALIGN_EXPAND);

	grid->addChild(label, Gui::ALIGN_LEFT);
	grid->addChild(h_box, Gui::ALIGN_LEFT);

	return ui;
}

AppSystemLogic::AppSystemLogic()
	: binder_(undo_stack_, [this]() { return decal_.get(); })
{}

AppSystemLogic::~AppSystemLogic()
{
}

int AppSystemLogic::init()
{
	assert(World::loadWorld("openair_bindings.world"));

	EngineWindowViewportPtr viewport = WindowManager::getMainWindow();
	assert(viewport.isValid());

	if (viewport->isFullscreen())
	{
		viewport->disableFullscreen();
		viewport->moveToCenter();
	}

	viewport->setTitle("Viewport");

	// Init parametes window
	auto parameters= EngineWindowViewport::create("Parameters", 512, 256);

	auto wrapper = WidgetScrollBox::create(parameters->getSelfGui());
	wrapper->setBackground(true);
	wrapper->setBorder(false);

	auto v_box = WidgetGridBox::create(parameters->getSelfGui(), 2, 2, 2);
	v_box->setBackground(true);
	v_box->setPadding(10, 10, 10, 10);
	v_box->setSpace(5, 5);

	width_ui_ = create_number_ui("Width", v_box);
	height_ui_ = create_number_ui("Height", v_box);

	wrapper->addChild(v_box, Gui::ALIGN_TOP | Gui::ALIGN_LEFT);
	parameters->addChild(wrapper, Gui::ALIGN_EXPAND);

	// Init layouts
	auto main = WindowManager::stackWindows(viewport, parameters,
		EngineWindowGroup::GROUP_TYPE_HORIZONTAL);


	// init bindings
	{
		auto binding = binder_.create<float>(&DecalOrtho::getWidth, &DecalOrtho::setWidth);
		binding->attach(width_ui_.edit_line);
		binding->attach(width_ui_.slider);
	}

	{
		auto binding = binder_.create<float>(&DecalOrtho::getHeight, &DecalOrtho::setHeight);
		binding->attach(height_ui_.edit_line);
		binding->attach(height_ui_.slider);
	}


	main->setTitle("Editor");
	main->setSize({1024, 512});
	main->moveToCenter();
	main->updateGuiHierarchy();
	main->setSeparatorValue(0, 0.7f);
	main->show();

	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// start of the main loop
////////////////////////////////////////////////////////////////////////////////

int AppSystemLogic::update()
{
	if (decal_.isNull())
	{
		decal_ = static_ptr_cast<DecalOrtho>(World::getNodeByName("decal"));
		assert(decal_.isValid());
	}

	binder_.update();

	if (Input::isKeyPressed(Input::KEY_LEFT_CTRL) && Input::isKeyDown(Input::KEY_Z))
	{
		undo_stack_.undo();
	}

	if (Input::isKeyPressed(Input::KEY_LEFT_CTRL) && Input::isKeyDown(Input::KEY_Y))
	{
		undo_stack_.redo();
	}

	// Write here code to be called before updating each render frame.
	return 1;
}

int AppSystemLogic::postUpdate()
{
	// Write here code to be called after updating each render frame.

	return 1;
}

////////////////////////////////////////////////////////////////////////////////
// end of the main loop
////////////////////////////////////////////////////////////////////////////////

int AppSystemLogic::shutdown()
{
	// Write here code to be called on engine shutdown.
	return 1;
}


