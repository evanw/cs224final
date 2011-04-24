#include "commands.h"
#include "document.h"

AddBallCommand::AddBallCommand(Document *doc, const Ball &ball) : ball(ball), doc(doc)
{
}

void AddBallCommand::undo()
{
    doc->mesh.balls.pop_back();
}

void AddBallCommand::redo()
{
    doc->mesh.balls += ball;
}

MoveBallCommand::MoveBallCommand(Document *doc, int index, const Vector3 &delta) : index(index), doc(doc)
{
    oldCenter = doc->mesh.balls[index].center;
    newCenter = oldCenter + delta;
}

void MoveBallCommand::undo()
{
    doc->mesh.balls[index].center = oldCenter;
}

void MoveBallCommand::redo()
{
    doc->mesh.balls[index].center = newCenter;
}

DeleteBallCommand::DeleteBallCommand(Document *doc, int index) : index(index), doc(doc), ball(doc->mesh.balls[index])
{
}

void DeleteBallCommand::undo()
{
    // add one to all indices >= index and connect child bones
    for (int i = 0; i < doc->mesh.balls.count(); i++)
    {
        Ball &ball = doc->mesh.balls[i];
        if (ball.parentIndex >= index)
            ball.parentIndex++;
    }
    foreach (int child, children)
        doc->mesh.balls[child].parentIndex = index;

    // can now modify the list
    doc->mesh.balls.insert(index, ball);
}

void DeleteBallCommand::redo()
{
    // subtract one from all indices > index and disconnect child bones
    for (int i = 0; i < doc->mesh.balls.count(); i++)
    {
        Ball &ball = doc->mesh.balls[i];
        if (ball.parentIndex == index)
        {
            ball.parentIndex = -1;
            children += (i > index) ? i - 1 : i;
        }
        else if (ball.parentIndex > index)
            ball.parentIndex--;
    }

    // can now modify the list
    doc->mesh.balls.removeAt(index);
}
