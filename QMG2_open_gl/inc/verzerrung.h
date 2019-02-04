struct CalibData{
    int FocusPointsMask;
    float PointsOrVec[4]; //Stores focus points or Normalized Vectors
    int A[2];
    float AngleOrLength[2];
};
struct CalibData* perspec_calibrating(int CalibPoints[7]);

