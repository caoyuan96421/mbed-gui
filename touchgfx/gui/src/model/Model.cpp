#include <gui/model/Model.hpp>
#include <gui/model/ModelListener.hpp>
#include <RotationAxis.h>

Model::Model() :
		modelListener(0), motorangle(0)
{
}

extern RotationAxis axis;

void Model::tick()
{
	motorangle = axis.getAngleDeg();
	if (modelListener)
		modelListener->setAngle(motorangle);
}
