#pragma once

#include <opencv2/opencv.hpp>

class Quaternion
{

public:
	Quaternion						();
	Quaternion						(const Quaternion &q);
	Quaternion						(const cv::Matx33f &mat);
	Quaternion						(const cv::Vec3f &euler);
	Quaternion						(const float &inR, const cv::Vec3f &inV);
	Quaternion						(const cv::Vec3f &axis, const float &angle);
	Quaternion						(const cv::Vec3f &from, const cv::Vec3f &to);

	void			set				(const float &inR, const cv::Vec3f &inV);
	const float&	operator[]		(int i) const;

	float			getR			() const;
	cv::Vec3f		getV			() const;
	float			getAngle		() const;
	cv::Vec3f		getAxis			() const;

	Quaternion		operator*		(const Quaternion &q) const;
	cv::Vec3f		operator*		(const cv::Vec3f &p) const;
	bool			operator==		(const Quaternion &oth) const;

	Quaternion		inverse			() const;

	cv::Matx33f		toMat			() const;
	cv::Vec3f		toEuler			() const;

	Quaternion		slerp			(Quaternion &q, float t) const;

	float			m_r;
	cv::Vec3f		m_v;
};

class Transform
{

public:
	Transform						();
	Transform						(const Transform &t);
	Transform						(const float &inScale, const cv::Vec3f &inTrans, const Quaternion &inRot);

	void			load			(const char *  fileName);
	void			save			(const char *  fileName);

	void			setTransform	(const float &inScale, const cv::Vec3f &inTrans, const Quaternion &inRot);
	void			setScale		(const float &inScale);
	void			setTrans		(const cv::Vec3f &inTrans);
	void			setRot			(const Quaternion &inRot);
	void			setCenter		(const cv::Vec3f &center);
	void			setDisplacement	(const cv::Vec3f &displacement);

	void			updateMat		();

	float			getScale		() const;
	cv::Vec3f		getTrans		() const;
	Quaternion		getRot			() const;

	cv::Vec3f		getCenter		() const;
	cv::Vec3f		getDisplacement () const;

	Transform		operator*		(const Transform &t) const;
	cv::Vec3f		operator*		(const cv::Vec3f &v) const;

	Transform		inverse			() const;

	cv::Matx44f		toMat			() const;

private:
	Quaternion		m_rot;
	float			m_scale;
	cv::Vec3f		m_trans;

	cv::Vec3f		m_center;

	cv::Matx44f		m_mat;

	cv::Vec3f		m_displacement;
};
