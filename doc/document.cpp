#include "document.h"
#include "commands.h"

void Document::moveBall(int ball, const Vector3 &delta)
{
    undoStack.push(new MoveBallCommand(ball, this, delta));
}
