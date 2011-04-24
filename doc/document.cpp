#include "document.h"
#include "commands.h"

void Document::addBall(const Ball &ball)
{
    undoStack.push(new AddBallCommand(this, ball));
}

void Document::moveBall(int ball, const Vector3 &delta)
{
    if (delta.lengthSquared() > 0)
        undoStack.push(new MoveBallCommand(this, ball, delta));
}

void Document::deleteBall(int ball)
{
    undoStack.push(new DeleteBallCommand(this, ball));
}
