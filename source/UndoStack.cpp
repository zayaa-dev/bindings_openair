#include "UndoStack.h"


UndoStack::~UndoStack()
{
	stack_.destroy();
}

void UndoStack::redo()
{
	if (index_ == stack_.size())
	{
		return;
	}

	UndoCommand *cmd = stack_.at(index_++);
	cmd->redo();
}

void UndoStack::undo()
{
	if (index_ == 0)
	{
		return;
	}

	UndoCommand *cmd = stack_.at(--index_);
	cmd->undo();
}

void UndoStack::push(UndoCommand *cmd)
{
	while (index_ < stack_.size())
	{
		delete stack_.takeLast();
	}

	stack_.push_back(cmd);
	cmd->redo();
	++index_;
}
