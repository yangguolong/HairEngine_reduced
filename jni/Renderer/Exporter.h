/*
		-= output mesh and image =-
stuff need to output:
1. headModel.obj		already exist in work directory
2. headImage.png		a composited image by poisson process
3. hairModel.obj		triangle strips to trangles
4. hairImage.png		4 channel rgba, need to balance the luminance
5. backMode.obj			background model
6. backImage.png		background image
7. trans.bin			store 3 matrices of type glm::mat4 in binary mode, inner order is head hair back

		-= Adjust hair to fit head =-
Function HairEngine::getFeatureMat will get all feature points for face;
use these point and pregenerated points to scale hair to adjust head :).

*/

#ifndef Exporter_H
#define Exporter_H
#include "IRenderer.h"
#include "glm.hpp"
#include "SurfaceModel.h"
#include "HeadModel.h"
#include "BackModel.h"
#include "ImageBalancer.h"
#include <opencv2/opencv.hpp>
#include <string>
#include <memory>

class Exporter : public IRenderer
{
public:
	Exporter(const std::string& workDir);

	void setHeadTextureData(const unsigned char * inputImageData, int width, int height) override;
	void setBackgroundTexutreData(const unsigned char * inputImageData, int width, int height) override;
	void setFaceImageData(const unsigned char * inputImageData, int width, int height) override;
	void initRenderer(int windowWidth, int windowHeight) override;
	void setRotation(float & xAngle, float & yAngle) override {}
	void setScale(float & scale) override {}
	void setHairPositionAdjustVector(float x, float y, float z) override;
	void setHairScale(float scale) override;
	void updateMatrixes() override;
	void render() override;
	void releaseMemory() override {}
	void changeHairStyle(const char * hairStyleDir) override;
	void resizeViewport(int windowWidth, int windowHeight) override {}
	unsigned char * takeScreenshot(int & width, int & height) override { return nullptr; }

	void enableExpression(bool isEnable) override { }
	void changeExpression(int expressionID) override { }
	void changeModifier(int modifierID) override { }
	void changeHairColor(float r, float g, float b, float a) override{ }
	void resetExpression() override{ }

	//void evaluateNewSurfaceTransData() override {}

private:
	void updateHeadMat();

	std::unique_ptr<HeadModel>		m_headModel;
	std::unique_ptr<SurfaceModel>	m_surfaceModel;
	std::unique_ptr<BackModel>		m_backModel;
	std::unique_ptr<ImageBalancer>	m_imageBalancer;

	glm::vec3						m_hairPositionAdjust{ 0.f };
	glm::vec3						m_hairScale{ 1.f };

	std::string						m_workDir;
	cv::Mat							m_headImage;
	cv::Mat							m_hairImage;
	cv::Mat							m_backImage;
	glm::mat4						m_headMat{ 1.f };
	glm::mat4						m_hairMat{ 1.f };
	glm::mat4						m_backMat{ 1.f };
};

#endif
