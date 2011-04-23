#ifndef SELECTIONRECORDER_H
#define SELECTIONRECORDER_H

class SelectionRecorder
{
private:
    bool depthTest;
    int viewport[4];
    double projectionMatrix[16];

public:
    SelectionRecorder();
    ~SelectionRecorder();

    // Enters selection mode at the given mouse coordinate.
    void enterSelectionMode(int x, int y);

    // Records any object drawn after this call as belonging to the object at index.
    // This uses glColor() so glColor() must not be used between entering and exiting selection mode.
    void setObjectIndex(int index);

    // Returns the index of the selected object, or -1 for no selection.
    int exitSelectionMode();
};

#endif // SELECTIONRECORDER_H
