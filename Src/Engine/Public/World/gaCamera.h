#ifndef GA_CAMERA_H
#define GA_CAMERA_H

#include "World/gaActor.h"

class gaCamera : public gaActor
{
	UE_DECLARE_CLASS(gaCamera, gaActor, 0);

public:
	virtual void Destroy();
	virtual void Update(f32 dt);
	virtual void SubmitRenderPackets(gxFrame* frame);
	virtual void Serialize(ueSerializer& s);
};

#endif // GA_CAMERA_H