#ifndef IRenderer_H
#define IRenderer_H

class IRenderer
{
public:
	virtual ~IRenderer() {}
	virtual void setHeadTextureData(const unsigned char * inputImageData, int width, int height) = 0;
	virtual void setBackgroundTexutreData(const unsigned char * inputImageData, int width, int height) = 0;
	virtual void setFaceImageData(const unsigned char * inputImageData, int width, int height) = 0;
	virtual void initRenderer(int windowWidth, int windowHeight) = 0;
	virtual void setRotation(float & xAngle, float & yAngle) = 0;
	virtual void setScale(float & scale) = 0;
	virtual void setHairPositionAdjustVector(float x, float y, float z) = 0;
	virtual void setHairScale(float scale) = 0;
	virtual void updateMatrixes() = 0;
	virtual void render() = 0;
	virtual void releaseMemory() = 0;
	virtual void changeHairStyle(const char * hairStyleDir) = 0;
	virtual void resizeViewport(int windowWidth, int windowHeight) = 0;
	virtual unsigned char * takeScreenshot(int & width, int & height) = 0;

	virtual void changeExpression(int expressionID) = 0;
	virtual void changeModifier(int modifierID) = 0;
	virtual void enableExpression(bool isEnable) = 0;
	virtual void changeHairColor(float r, float g, float b, float a) = 0;
	virtual void resetExpression() = 0;
	//virtual void evaluateNewSurfaceTransData() = 0;
};

#endif
