#pragma once

#include <UnigineVector.h>

class UndoCommand
{
public:
	virtual ~UndoCommand() = default;
	virtual void undo() = 0;
	virtual void redo() = 0;
};

class UndoStack final
{
public:
	~UndoStack();

	void redo();
	void undo();
	void push(UndoCommand *cmd);

private:
	int index_{0};
	Unigine::Vector<UndoCommand *> stack_;
};
