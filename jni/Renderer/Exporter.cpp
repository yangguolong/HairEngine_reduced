#include "Exporter.h"
#include "matrix_transform.hpp"
#include "type_ptr.hpp"

#include <fstream>

#include <android/log.h>
#define LOG_TAG "Exporter"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG, ##__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG, ##__VA_ARGS__)

Exporter::Exporter(const std::string& workDir) :
m_workDir(workDir)
{
	LOGI("Ctor");
}

void Exporter::setHeadTextureData(const unsigned char * inputImageData, int width, int height)
{
	LOGI("setHeadTextureData");
	cv::Mat headImage{ height, width, CV_8UC4, const_cast<unsigned char*>(inputImageData) };
	cv::cvtColor(headImage, m_headImage, CV_BGRA2RGBA);
}

void Exporter::setBackgroundTexutreData(const unsigned char * inputImageData, int width, int height)
{
	LOGI("setBackgroundTexutreData");
	cv::Mat backImage{ height, width, CV_8UC4, const_cast<unsigned char*>(inputImageData) };
	cv::cvtColor(backImage, m_backImage, CV_BGRA2RGBA);
}

void Exporter::setFaceImageData(const unsigned char * inputImageData, int width, int height)
{
	LOGI("setFaceImageData");
	m_imageBalancer = std::unique_ptr<ImageBalancer>(new ImageBalancer);
	m_imageBalancer->initBalancer(inputImageData, width, height);
}

void Exporter::initRenderer(int windowWidth, int windowHeight)
{
	LOGI("initRenderer");
	// load head model
	m_headModel = std::unique_ptr<HeadModel>(new HeadModel);
	auto ret = m_headModel->loadModelFromFile(
		(m_workDir + "head.obj").c_str(),
		(m_workDir + "trans.txt").c_str()
		);
	if (!ret) {
		LOGE("fail to load head model");
		return;
	}
	updateHeadMat();
	//load back model
	m_backModel = std::unique_ptr<BackModel>(new BackModel);
	ret = m_backModel->loadBackModel(m_backImage.cols, m_backImage.rows, (m_workDir + "boundary.txt").c_str(), m_headModel.get(), m_headMat);
	if (!ret) {
		LOGE("fail to load back model");
		return;
	}
	m_backModel->runDeform(m_headMat);
}

void Exporter::setHairPositionAdjustVector(float x, float y, float z)
{
	LOGI("setHairPositionAdjustVector");
	m_hairPositionAdjust = glm::vec3{ x, y, z };

}

void Exporter::setHairScale(float scale)
{
	m_hairScale = glm::vec3{ scale };
}

void Exporter::updateMatrixes()
{
	LOGI("updateMatrixes");
	//Transform matrix
	BoundBox * headBoundBox = m_headModel->getBoundBox();
	TransformMatrix * transformMatrix = m_headModel->getTransformMatrix();
	float scaleFactor = (*transformMatrix).scaleFactor;
	float xTrans = (*transformMatrix).positionTrans[0];
	float yTrans = (*transformMatrix).positionTrans[1];
	float zTrans = (*transformMatrix).positionTrans[2];
	float longMoveX = (*headBoundBox).boundCenter[0] * scaleFactor + xTrans;
	float longMoveY = (*headBoundBox).boundCenter[1] * scaleFactor + yTrans;
	float longMoveZ = (*headBoundBox).boundCenter[2] * scaleFactor + zTrans + (((*headBoundBox).boundMax[2] - (*headBoundBox).boundMin[2]) / 2.0f * 0.4);

	glm::mat4x4 baseModelViewMatrix = glm::translate(glm::mat4x4(1.0f), glm::vec3(longMoveX, longMoveY, longMoveZ));
	baseModelViewMatrix = glm::translate(baseModelViewMatrix, glm::vec3(-longMoveX, -longMoveY, -longMoveZ));

	m_headMat = glm::translate(baseModelViewMatrix, glm::vec3(xTrans, yTrans, zTrans));
	m_headMat = glm::scale(m_headMat, glm::vec3(scaleFactor));

	glm::mat4x4 hairAdjustMatrix = glm::translate(glm::mat4x4(1.0f), glm::vec3(longMoveX, longMoveY, longMoveZ));
	hairAdjustMatrix = glm::translate(hairAdjustMatrix, m_hairPositionAdjust);
	hairAdjustMatrix = glm::scale(hairAdjustMatrix, m_hairScale);
	hairAdjustMatrix = glm::translate(hairAdjustMatrix, glm::vec3(-longMoveX, -longMoveY, -longMoveZ));

	TransformMatrix * matrix = nullptr;
	float * rotation = nullptr;
	float * position = nullptr;
	float scale = 1.f;
	float tmpMat[16] {0.f};

	glm::mat4x4 backMatrix{ 1.f };

	if (m_surfaceModel) {
//		LOGI("surfaceModel is not nullptr");
		matrix = m_surfaceModel->getTransformMatrix();
		rotation = (*matrix).rotation;
		position = (*matrix).positionTrans;
		scale = (*matrix).scaleFactor;

		tmpMat[0] = rotation[0];    tmpMat[1] = rotation[1];    tmpMat[2] = rotation[2];    tmpMat[3] = 0.0f;
		tmpMat[4] = rotation[3];    tmpMat[5] = rotation[4];    tmpMat[6] = rotation[5];    tmpMat[7] = 0.0f;
		tmpMat[8] = rotation[6];    tmpMat[9] = rotation[7];    tmpMat[10] = rotation[8];   tmpMat[11] = 0.0f;
		tmpMat[12] = 0.0f;          tmpMat[13] = 0.0f;          tmpMat[14] = 0.0f;          tmpMat[15] = 1.0f;
		backMatrix = glm::make_mat4x4(tmpMat);

		scale = 1.0 / scale;
		backMatrix = glm::scale(backMatrix, glm::vec3(scale));
		backMatrix = glm::translate(backMatrix, glm::vec3(-position[0], -position[1], -position[2]));
	} else {
//		LOGI("surfaceModel is nullptr");
	}

	matrix = m_headModel->getTransformMatrix();
	rotation = (*matrix).rotation;
	position = (*matrix).positionTrans;
	scale = (*matrix).scaleFactor;

	glm::mat4x4 forwardMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(position[0], position[1], position[2]));
	forwardMatrix = glm::scale(forwardMatrix, glm::vec3(scale));

	tmpMat[0] = rotation[0];    tmpMat[1] = rotation[1];    tmpMat[2] = rotation[2];    tmpMat[3] = 0.0f;
	tmpMat[4] = rotation[3];    tmpMat[5] = rotation[4];    tmpMat[6] = rotation[5];    tmpMat[7] = 0.0f;
	tmpMat[8] = rotation[6];    tmpMat[9] = rotation[7];    tmpMat[10] = rotation[8];   tmpMat[11] = 0.0f;
	tmpMat[12] = 0.0f;          tmpMat[13] = 0.0f;          tmpMat[14] = 0.0f;          tmpMat[15] = 1.0f;
	glm::mat4x4 rotationMatrix = glm::make_mat4x4(tmpMat);

	rotationMatrix = glm::inverse(rotationMatrix);
	forwardMatrix = forwardMatrix * rotationMatrix;
	glm::mat4x4 displancementMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, m_headModel->getDisplacementY(), m_headModel->getDisplacementZ()));

	m_hairMat = baseModelViewMatrix * hairAdjustMatrix * forwardMatrix * displancementMatrix * backMatrix;
}

void Exporter::render()
{
	LOGI("render");

	// Export head and back mesh/image/matrix
	// Export matrix as a package
	std::string path;

	// output head image
	path = m_workDir + "headImage.png";
	LOGI("%s", path.c_str());
	cv::imwrite(path, m_headImage);

	// output back image
	path = m_workDir + "backImage.png";
	LOGI("%s", path.c_str());
	cv::imwrite(path, m_backImage);

	// output head and hair transform matrix
	updateMatrixes();
	path = m_workDir + "trans.bin";
	LOGI("%s", path.c_str());
	std::ofstream transOfs{ path, std::ios::binary };
	transOfs.write(reinterpret_cast<char*>(glm::value_ptr(m_headMat)), sizeof(glm::mat4));
	transOfs.write(reinterpret_cast<char*>(glm::value_ptr(m_hairMat)), sizeof(glm::mat4));
	transOfs.write(reinterpret_cast<char*>(glm::value_ptr(m_backMat)), sizeof(glm::mat4));

	// output back mesh
	path = m_workDir + "back.obj";
	LOGI("%s", path.c_str());
	std::ofstream backOfs{ path };
	auto vertexArray = reinterpret_cast<glm::vec3*>(m_backModel->getPositionArray());
	auto uvArray = reinterpret_cast<glm::vec2*>(m_backModel->getUVArray());
	for (glm::uint i = 0; i < m_backModel->getVertexNum(); i++) {
		const auto& vertex = vertexArray[i];
		const auto& uv = uvArray[i];
		backOfs << "v " << vertex.x << ' ' << vertex.y << ' ' << vertex.z << std::endl;
		backOfs << "vt " << uv.x << ' ' << uv.y << std::endl;
	}
	auto indexArray = reinterpret_cast<glm::uvec3*>(m_backModel->getIndexArray());
	for (glm::uint i = 0; i < m_backModel->getIndexNum() / 3; i++) {
		auto face = indexArray[i] + glm::uvec3(1);
		backOfs << "f "
			<< face.x << '/' << face.x << ' '
			<< face.y << '/' << face.y << ' '
			<< face.z << '/' << face.z << ' '
			<< std::endl;
	}

}

void Exporter::changeHairStyle(const char * hairStyleDir)
{
	// Export hair mesh/image/matrix
	// Export matrix as a package

	LOGI("changeHairStyle");
	if (!hairStyleDir) {
		LOGE("hairStyleDir is nullptr");
		return;
	}

	std::string dir{ hairStyleDir };

	m_surfaceModel = std::unique_ptr<SurfaceModel>(new SurfaceModel);
	auto ret = m_surfaceModel->loadModelFromDepthFile(
		(dir + "/depth.png").c_str(),
		(dir + "/depthInfo.txt").c_str(),
		(dir + "/trans.txt").c_str()
		);
	if (!ret) {
		LOGE("fail to loadModelFromDepthFile");
		return;
	}

	std::ifstream additionOfs{ dir + "/addition.txt" };
	if (!additionOfs.good()) {
		LOGE("fail to load addition.txt");
		return;
	}
	float luminance = 0.f;
	additionOfs >> luminance;
	/*
	TODO
	input scale factor and update m_hairScale,
	this will update matrix
	*/
	
	auto rgb = cv::imread(dir + "/hairTexRGB.jpg");
	auto a = cv::imread(dir + "/hairTexA.jpg");
	cv::Mat channels[3];
	cv::split(a, channels);
	channels[0] = rgb;
	cv::merge(channels, 2, m_hairImage);

	m_imageBalancer->runBalance(luminance, m_hairImage.ptr(), m_hairImage.cols, m_hairImage.rows);

	std::string path;

	// output hair image
	path = m_workDir + "hairImage.png";
	LOGI("%s", path.c_str());
	cv::imwrite(path, m_hairImage);

	// output head and hair transform matrix
	updateMatrixes();
	path = m_workDir + "trans.bin";
	LOGI("%s", path.c_str());
	std::ofstream transOfs{ path, std::ios::binary };
	transOfs.write(reinterpret_cast<char*>(glm::value_ptr(m_headMat)), sizeof(glm::mat4));
	transOfs.write(reinterpret_cast<char*>(glm::value_ptr(m_hairMat)), sizeof(glm::mat4));
	transOfs.write(reinterpret_cast<char*>(glm::value_ptr(m_backMat)), sizeof(glm::mat4));

	// output hair mesh
	path = m_workDir + "hair.obj";
	LOGI("%s", path.c_str());
	std::ofstream hairOfs{ path };
	auto halfWidth = m_hairImage.cols / 2.f;
	auto vertexArray = reinterpret_cast<glm::vec3*>(m_surfaceModel->getVertexDataArray());
	for (glm::uint i = 0; i < m_surfaceModel->getSortedVertexNum(); i++) {
		const auto& vertex = vertexArray[i];
		if (vertex.x > halfWidth)
			hairOfs << "v " << vertex.x - halfWidth << ' ' << vertex.y << ' ' << vertex.z << std::endl;
		else
			hairOfs << "v " << vertex.x << ' ' << vertex.y << ' ' << vertex.z << std::endl;
		hairOfs << "vt " << vertex.x / m_hairImage.cols << ' ' << vertex.y / m_hairImage.rows << std::endl;
	}
	for (glm::uint i = 2; i < m_surfaceModel->getSortedVertexNum(); i++) {
		glm::uvec3 topo;
		if (0 == i % 2) topo = glm::uvec3{ i - 1, i, i + 1 };	// ccw
		else topo = glm::uvec3{ i, i - 1, i + 1 };				// cw
		hairOfs << "f "
			<< topo.x << '/' << topo.x << ' '
			<< topo.y << '/' << topo.y << ' '
			<< topo.z << '/' << topo.z << ' '
			<< std::endl;
	}
}

void Exporter::updateHeadMat()
{
	//Transform matrix
	BoundBox * headBoundBox = m_headModel->getBoundBox();
	TransformMatrix * transformMatrix = m_headModel->getTransformMatrix();
	float scaleFactor = (*transformMatrix).scaleFactor;
	float xTrans = (*transformMatrix).positionTrans[0];
	float yTrans = (*transformMatrix).positionTrans[1];
	float zTrans = (*transformMatrix).positionTrans[2];
	float longMoveX = (*headBoundBox).boundCenter[0] * scaleFactor + xTrans;
	float longMoveY = (*headBoundBox).boundCenter[1] * scaleFactor + yTrans;
	float longMoveZ = (*headBoundBox).boundCenter[2] * scaleFactor + zTrans + (((*headBoundBox).boundMax[2] - (*headBoundBox).boundMin[2]) / 2.0f * 0.4);

	glm::mat4x4 baseModelViewMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(longMoveX, longMoveY, longMoveZ));
	baseModelViewMatrix = glm::translate(baseModelViewMatrix, glm::vec3(-longMoveX, -longMoveY, -longMoveZ));

	m_headMat = glm::translate(baseModelViewMatrix, glm::vec3(xTrans, yTrans, zTrans));
	m_headMat = glm::scale(m_headMat, glm::vec3(scaleFactor));
}


//void Exporter::output()
//{
//	std::string path;
//
//	// output head image
//	path = m_workDir + "headImage.png";
//	LOGI("%s", path.c_str());
//	cv::imwrite(path, m_headImage);
//
//	// output hair image
//	path = m_workDir + "hairImage.png";
//	LOGI("%s", path.c_str());
//	cv::imwrite(path, m_hairImage);
//
//	// output back image
//	path = m_workDir + "backImage.png";
//	LOGI("%s", path.c_str());
//	cv::imwrite(path, m_backImage);
//
//	// output head and hair transform matrix
//	path = m_workDir + "trans.bin";
//	LOGI("%s", path.c_str());
//	std::ofstream transOfs{ path, std::ios::binary };
//	transOfs.write(reinterpret_cast<char*>(glm::value_ptr(m_headMat)), sizeof(glm::mat4));
//	transOfs.write(reinterpret_cast<char*>(glm::value_ptr(m_hairMat)), sizeof(glm::mat4));
//	transOfs.write(reinterpret_cast<char*>(glm::value_ptr(m_backMat)), sizeof(glm::mat4));
//
//	// output hair mesh
//	path = m_workDir + "hair.obj";
//	LOGI("%s", path.c_str());
//	std::ofstream hairOfs{ path };
//	auto halfWidth = m_hairImage.cols / 2.f;
//	auto vertexArray = reinterpret_cast<glm::vec3*>(m_surfaceModel->getVertexDataArray());
//	for (glm::uint i = 0; i < m_surfaceModel->getSortedVertexNum(); i++) {
//		const auto& vertex = vertexArray[i];
//		if (vertex.x > halfWidth)
//			hairOfs << "v " << vertex.x - halfWidth << ' ' << vertex.y << ' ' << vertex.z << std::endl;
//		else
//			hairOfs << "v " << vertex.x << ' ' << vertex.y << ' ' << vertex.z << std::endl;
//		hairOfs << "vt " << vertex.x / m_hairImage.cols << ' ' << vertex.y / m_hairImage.rows << std::endl;
//	}
//	for (glm::uint i = 2; i < m_surfaceModel->getSortedVertexNum(); i++) {
//		glm::uvec3 topo;
//		if (0 == i % 2) topo = glm::uvec3{ i - 1, i, i + 1 };	// ccw
//		else topo = glm::uvec3{ i, i - 1, i + 1 };				// cw
//		hairOfs << "f "
//			<< topo.x << '/' << topo.x << ' '
//			<< topo.y << '/' << topo.y << ' '
//			<< topo.z << '/' << topo.z << ' '
//			<< std::endl;
//	}
//
//	// output back mesh
//	path = m_workDir + "back.obj";
//	LOGI("%s", path.c_str());
//	std::ofstream backOfs{ path };
//	vertexArray = reinterpret_cast<glm::vec3*>(m_backModel->getPositionArray());
//	auto uvArray = reinterpret_cast<glm::vec2*>(m_backModel->getUVArray());
//	for (glm::uint i = 0; i < m_backModel->getVertexNum(); i++) {
//		const auto& vertex = vertexArray[i];
//		const auto& uv = uvArray[i];
//		backOfs << "v " << vertex.x << ' ' << vertex.y << ' ' << vertex.z << std::endl;
//		backOfs << "vt " << uv.x << ' ' << uv.y << std::endl;
//	}
//	auto indexArray = reinterpret_cast<glm::uvec3*>(m_backModel->getIndexArray());
//	for (glm::uint i = 0; i < m_backModel->getIndexNum() / 3; i++) {
//		auto face = indexArray[i] + glm::uvec3(1);
//		backOfs << "f "
//			<< face.x << '/' << face.x << ' '
//			<< face.y << '/' << face.y << ' '
//			<< face.z << '/' << face.z << ' '
//			<< std::endl;
//	}
//
//}
