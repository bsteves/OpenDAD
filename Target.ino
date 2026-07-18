/*
  Target.ino

  Stores the target angle selected from the OpenDAD web interface.
*/

float targetAngle = 0.0f;
bool targetAngleSet = false;


void setTargetAngle(float angle)
{
  targetAngle = angle;
  targetAngleSet = true;

  Serial.print("Target angle set to ");
  Serial.print(targetAngle, 2);
  Serial.println(" degrees");
}


void setTargetToCurrentAngle()
{
  setTargetAngle(smoothedAngle);
}


void clearTargetAngle()
{
  targetAngle = 0.0f;
  targetAngleSet = false;

  Serial.println("Target angle cleared");
}


float getTargetDifference()
{
  if (!targetAngleSet)
  {
    return 0.0f;
  }

  /*
    Negative = current angle is below target.
    Positive = current angle is above target.
  */
  return smoothedAngle - targetAngle;
}