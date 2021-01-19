#pragma once
#include "Component.h"
#include "Math/Matrix.h"

namespace Seele
{
class CameraComponent : public Component
{
public:
    CameraComponent();
    virtual ~CameraComponent();

	Matrix4 getViewMatrix()
	{
		if (bNeedsViewBuild)
		{
			buildViewMatrix();
		}
		return viewMatrix;
	}
	Matrix4 getProjectionMatrix()
	{
		if (bNeedsProjectionBuild)
		{
			buildProjectionMatrix();
		}
		return projectionMatrix;
	}
	Vector getCameraPosition()
	{
		if(bNeedsViewBuild)
		{
			buildViewMatrix();
		}
		return cameraPosition;
	}
	void mouseMove(float deltaX, float deltaY);
	void mouseScroll(double x);
	void moveOrigin(float up);
	float aspectRatio;
	float fieldOfView;
private:
    bool bNeedsViewBuild;
	bool bNeedsProjectionBuild;
	void buildViewMatrix();
	void buildProjectionMatrix();

	float rotationX;
	float rotationY;
	float distance;

	Vector eye;
	Vector originPoint;
	Vector cameraPosition;
	//Transforms relative to actor
	Matrix4 viewMatrix;
	Matrix4 projectionMatrix;
};
DEFINE_REF(CameraComponent);
} // namespace Seele
