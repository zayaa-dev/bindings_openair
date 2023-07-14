#pragma once

#include "Common.h"
#include "FunctionTraits.h"
#include "UndoStack.h"

#include <UnigineWidgets.h>


namespace binds
{

class IBinding
{
public:
	virtual ~IBinding() = default;
	virtual void update() = 0;
	virtual IBinding *attach(Unigine::WidgetPtr widget) = 0;
};

template<typename RetT, typename ArgT>
class IModel
{
public:
	virtual ~IModel() = default;
	virtual RetT get() const = 0;
	virtual bool set(ArgT) = 0;

	virtual void startUpdating() = 0;
	virtual void finishUpdating() = 0;
	virtual void cancelUpdating() = 0;
	virtual bool isUpdating() const = 0;
};

class IView
{
public:
	virtual ~IView() = default;
	virtual void update() = 0;

protected:
	IView() = default;
};

template<typename RetT, typename ArgT>
class BindingTemplate : public IBinding
{
public:
	using Model = IModel<RetT, ArgT>;

	BindingTemplate(Model *model)
		: model_(model)
	{}

	~BindingTemplate() override { delete model_; }

	virtual RetT get() const { return model_->get(); }
	virtual void set(ArgT v)
	{
		if (model_->set(v))
		{
			update();
		}
	}

	void update() override
	{
		for (const auto &view : views_)
		{
			view->update();
		}
	}

	void startUpdating()
	{
		if (!model_->isUpdating())
		{
			model_->startUpdating();
		}
	}
	void finishUpdating()
	{
		if (model_->isUpdating())
		{
			model_->finishUpdating();
		}
	}
	void cancelUpdating()
	{
		if (model_->isUpdating())
		{
			model_->cancelUpdating();
		}
	}
	bool isUpdating() const { return model_->isUpdating(); }

protected:
	Model *model_{};
	Unigine::Vector<IView *> views_;
};

// primary binding template
template<typename T>
class Binding
{};


template<typename T>
class WidgetEditLineView final : public IView
{
public:
	WidgetEditLineView(Unigine::WidgetEditLinePtr w, Binding<T> *b)
		: w_(w)
		, b_(b)
	{

		Unigine::CallbackBase *cb = Unigine::MakeCallback([this]() {
			b_->startUpdating();
		});

		start_edit_callback_ = w_->addCallback(Unigine::Gui::FOCUS_IN, cb);

		cb = Unigine::MakeCallback([this]() { w_->removeFocus(); });

		pressed_callback_ = w_->addCallback(Unigine::Gui::PRESSED, cb);

		cb = Unigine::MakeCallback([this]() {
			b_->finishUpdating();
		});

		finish_edit_callback_ = w_->addCallback(Unigine::Gui::FOCUS_OUT, cb);

		cb = Unigine::MakeCallback([this]() {
			const double valued = Unigine::String::atod(w_->getText());
			b_->set(valued);
		});

		changed_callback_ = w_->addCallback(Unigine::Gui::CHANGED, cb);
	}

	~WidgetEditLineView() override
	{
		if (w_)
		{
			w_->removeCallback(Unigine::Gui::FOCUS_IN, start_edit_callback_);
			w_->removeCallback(Unigine::Gui::FOCUS_OUT, finish_edit_callback_);
			w_->removeCallback(Unigine::Gui::PRESSED, pressed_callback_);
			w_->removeCallback(Unigine::Gui::CHANGED, changed_callback_);
		}
	}

	void update() override
	{
		if (w_.isNull() || w_->isFocused())
		{
			return;
		}

		auto value = b_->get();

		if (compare(Unigine::String::atof(w_->getText()), value))
		{
			return;
		}

		w_->setCallbackEnabled(Unigine::Gui::CHANGED, false);
		w_->setText(Unigine::String::format("%.3f", value));
		w_->setCallbackEnabled(Unigine::Gui::CHANGED, true);
	}

private:
	void *start_edit_callback_{};
	void *pressed_callback_{};
	void *finish_edit_callback_{};
	void *changed_callback_{};

	Unigine::WidgetEditLinePtr w_;
	Binding<T> *b_{};
};

template<typename T>
class SliderView final : public IView
{
public:
	SliderView(Unigine::WidgetSliderPtr w, Binding<T> *b)
		: w_(w)
		, b_(b)
	{
		Unigine::CallbackBase *cb = Unigine::MakeCallback([this]() {
			b_->startUpdating();
			is_editing_ = true;
		});

		start_edit_callback_ = w_->addCallback(Unigine::Gui::PRESSED, cb);

		cb = Unigine::MakeCallback([this]() {
			b_->finishUpdating();
			is_editing_ = false;
		});

		finish_edit_callback_ = w_->addCallback(Unigine::Gui::RELEASED, cb);

		cb = Unigine::MakeCallback([this]() {
			b_->set(remap(w_->getMinValue(), w_->getMaxValue(), 0.0, 5.0, w_->getValue()));
		});

		changed_callback_ = w_->addCallback(Unigine::Gui::CHANGED, cb);
	}

	~SliderView() override
	{
		if (w_)
		{
			w_->removeCallback(Unigine::Gui::PRESSED, start_edit_callback_);
			w_->removeCallback(Unigine::Gui::RELEASED, finish_edit_callback_);
			w_->removeCallback(Unigine::Gui::CHANGED, changed_callback_);
		}
	}

	void update() override
	{
		if (is_editing_)
		{
			return;
		}

		auto value = b_->get();

		w_->setCallbackEnabled(Unigine::Gui::CHANGED, false);
		w_->setValue(remap(0.0, 5.0, w_->getMinValue(), w_->getMaxValue(), value));
		w_->setCallbackEnabled(Unigine::Gui::CHANGED, true);
	}

private:
	double remap(double in_min, double in_max, double out_min, double out_max, double in_v)
	{
		using namespace Unigine::Math;
		return lerp(out_min, out_max, inverseLerp(in_min, in_max, in_v));
	}

	void *start_edit_callback_{};
	void *finish_edit_callback_{};
	void *changed_callback_{};

	bool is_editing_{false};
	Unigine::WidgetSliderPtr w_;
	Binding<T> *b_{};
};

template<>
class Binding<float> final : public BindingTemplate<float, float>
{
public:
	explicit Binding(Model *model)
		: BindingTemplate<float, float>(model)
	{}

	IBinding *attach(Unigine::WidgetPtr w) override
	{
		if (auto edit_line = Unigine::checked_ptr_cast<Unigine::WidgetEditLine>(w))
		{
			return attach(edit_line);
		}
		else if (auto slider = Unigine::checked_ptr_cast<Unigine::WidgetSlider>(w))
		{
			return attach(slider);
		}

		assert(false); // or log error/fatal
		return this;
	}

	Binding<float> *attach(Unigine::WidgetEditLinePtr w)
	{
		auto view = new WidgetEditLineView<float>(w, this);
		views_.append(view);
		return this;
	}

	Binding<float> *attach(Unigine::WidgetSliderPtr w)
	{
		auto view = new SliderView<float>(w, this);
		views_.append(view);
		return this;
	}
};

template<typename InstanceT, auto Getter, auto Setter,
		 typename RetT = typename function_traits<function_signature<Getter>>::result_type,
		 typename ArgT = typename function_traits<function_signature<Setter>>::template arg<0>::type>
class UndoRedoModel final : public IModel<RetT, ArgT>
{
public:
	using T = RetT;

	using InstanceGetter = std::function<InstanceT *()>;
	UndoRedoModel(UndoStack &undo, InstanceGetter instance_getter)
		: undo_stack_(undo)
		, instance_getter_(instance_getter)
	{}

	RetT get() const override
	{
		return (instance_getter_()->*Getter)();
	}

	bool set(ArgT v) override
	{
		if (compare((instance_getter_()->*Getter)(), v))
		{
			return false;
		}

		if (isUpdating())
		{
			transaction_->update(v);
			transaction_->redo();
		}
		else
		{
			undo_stack_.push(new Transaction(instance_getter_()));
		}
		return true;
	}

	void startUpdating() override
	{
		if (isUpdating())
		{
			return;
		}

		transaction_ = new Transaction(instance_getter_());
	}

	void finishUpdating() override
	{
		if (!isUpdating())
		{
			return;
		}

		if (transaction_->hasModifications())
		{
			undo_stack_.push(transaction_);
		}
		else
		{
			delete transaction_;
		}

		transaction_ = nullptr;
	}

	void cancelUpdating() override
	{
		if (!isUpdating())
		{
			return;
		}

		transaction_->undo();
		delete transaction_;
		transaction_ = nullptr;
	}

	bool isUpdating() const override { return transaction_; }

private:
	class Transaction final : public UndoCommand
	{
	public:
		Transaction(InstanceT *instance)
			: instance_(instance)
			, old_value_((instance->*Getter)())
			, new_value_(old_value_)
		{}

		void update(ArgT v) { new_value_ = v; }

		bool hasModifications() const { return !compare(new_value_, old_value_); }

		void redo() override
		{
			(instance_->*Setter)(new_value_);
		}

		void undo() override
		{
			(instance_->*Setter)(old_value_);
		}

	private:
		InstanceT *instance_{};
		RetT old_value_;
		RetT new_value_;
	};

	UndoStack &undo_stack_;
	InstanceGetter instance_getter_;
	Transaction *transaction_{nullptr};
};

template<typename InstanceT>
class Binder
{
public:
	using InstanceGetter = std::function<InstanceT *()>;

	template <typename T>
	using Getter = std::function<T(InstanceT *)>;

	template <typename T>
	using Setter = std::function<void(InstanceT *, const T &)>;


	Binder(UndoStack &undo_stack, InstanceGetter instance_getter)
		: undo_stack_(undo_stack)
		, instance_getter_(instance_getter)
	{}

	template<auto Getter, auto Setter>
	auto create()
	{
		using Model = UndoRedoModel<InstanceT, Getter, Setter>;

		auto model = new Model(undo_stack_, instance_getter_);

		auto binding = new Binding<typename Model::T>(model);
		bindings_.append(binding);
		return binding;
	}

	void update()
	{
		for (const auto &binding : bindings_)
		{
			binding->update();
		}
	}

private:
	UndoStack &undo_stack_;
	InstanceGetter instance_getter_;
	Unigine::Vector<IBinding *> bindings_;
};

}
