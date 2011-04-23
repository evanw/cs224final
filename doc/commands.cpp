#include "commands.h"
#include "document.h"

MoveBallCommand::MoveBallCommand(int ball, Document *doc, const Vector3 &delta) : ball(ball), doc(doc)
{
    oldCenter = doc->mesh.balls[ball].center;
    newCenter = oldCenter + delta;
}

void MoveBallCommand::undo()
{
    doc->mesh.balls[ball].center = oldCenter;
}

void MoveBallCommand::redo()
{
    doc->mesh.balls[ball].center = newCenter;
}
