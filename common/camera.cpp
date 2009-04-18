#include "lc_global.h"
#include "camera.h"

#include "opengl.h"
#include "defines.h"
#include "matrix.h"
#include "file.h"
#include "lc_application.h"
#include "lc_colors.h"
#include "tr.h"

#define LC_CAMERA_SAVE_VERSION 6 // LeoCAD 0.73

static LC_OBJECT_KEY_INFO camera_key_info[LC_CK_COUNT] =
{
	{ "Camera Position", 3, LC_CK_EYE },
	{ "Camera Target", 3, LC_CK_TARGET },
	{ "Camera Roll", 1, LC_CK_ROLL }
};

// =============================================================================
// CameraTarget class

CameraTarget::CameraTarget(lcCamera* Parent)
	: lcObject(LC_OBJECT_CAMERA_TARGET)
{
	m_Parent = Parent;
	m_Name = Parent->m_Name + ".Target";
}

CameraTarget::~CameraTarget()
{
}

void CameraTarget::ClosestLineIntersect(lcClickLine& ClickLine) const
{
	BoundingBox Box;
	Box.m_Max = Vector3(0.2f, 0.2f, 0.2f);
	Box.m_Min = Vector3(-0.2f, -0.2f, -0.2f);

	Matrix44 WorldView = ((lcCamera*)m_Parent)->m_WorldView;
	WorldView.SetTranslation(Mul30(-((lcCamera*)m_Parent)->m_TargetPosition, WorldView));

	Vector3 Start = Mul31(ClickLine.Start, WorldView);
	Vector3 End = Mul31(ClickLine.End, WorldView);

	float Dist;
	if (BoundingBoxRayMinIntersectDistance(Box, Start, End, &Dist) && (Dist < ClickLine.Dist))
	{
		ClickLine.Object = this;
		ClickLine.Dist = Dist;
	}
}

bool CameraTarget::IntersectsVolume(const Vector4* Planes, int NumPlanes) const
{
	BoundingBox Box;
	Box.m_Max = Vector3(0.2f, 0.2f, 0.2f);
	Box.m_Min = Vector3(-0.2f, -0.2f, -0.2f);

	// Transform the planes to local space.
	Vector4* LocalPlanes = new Vector4[NumPlanes];
	int i;

	Matrix44 WorldView = ((lcCamera*)m_Parent)->m_WorldView;
	WorldView.SetTranslation(Mul30(-((lcCamera*)m_Parent)->m_TargetPosition, WorldView));

	for (i = 0; i < NumPlanes; i++)
	{
		LocalPlanes[i] = Vector4(Mul30(Vector3(Planes[i]), WorldView));
		LocalPlanes[i][3] = Planes[i][3] - Dot3(Vector3(WorldView[3]), Vector3(LocalPlanes[i]));
	}

	bool Intersect = BoundingBoxIntersectsVolume(Box, LocalPlanes, NumPlanes);

	delete[] LocalPlanes;

	return Intersect;
}

void CameraTarget::Select(bool bSelecting, bool bFocus, bool bMultiple)
{
	m_Parent->SelectTarget(bSelecting, bFocus, bMultiple);
}

/////////////////////////////////////////////////////////////////////////////
// Camera construction/destruction

lcCamera::lcCamera()
	: lcObject(LC_OBJECT_CAMERA)
{
	Initialize();
}

// Start with a standard camera.
lcCamera::lcCamera(unsigned char nType, lcCamera* pPrev)
	: lcObject(LC_OBJECT_CAMERA)
{
	if (nType > 7)
		nType = 8;

	char names[8][7] = { "Front", "Back",  "Top",  "Under", "Left", "Right", "Main", "User" };
	float eyes[8][3] = { { 50,0,0 }, { -50,0,0 }, { 0,0,50 }, { 0,0,-50 },
	                     { 0,50,0 }, { 0,-50,0 }, { -10,-10,5}, { 0,5,0 } };
	float roll = 0.0f;

	Initialize();

	ChangeKey(1, false, true, eyes[nType], LC_CK_EYE);
	ChangeKey(1, false, true, &roll, LC_CK_ROLL);
	ChangeKey(1, true, true, eyes[nType], LC_CK_EYE);
	ChangeKey(1, true, true, &roll, LC_CK_ROLL);

	m_Name = names[nType];
	if (nType != 8)
		m_nState = LC_CAMERA_HIDDEN;
	m_nType = nType;

	if (pPrev)
		pPrev->m_Next = this;

	UpdatePosition(1, false);
}

// From OnMouseMove(), case LC_ACTION_ROTATE_VIEW
lcCamera::lcCamera(const float *eye, const float *target, float roll, lcObject* pCamera)
	: lcObject(LC_OBJECT_CAMERA)
{
	Initialize();

	ChangeKey(1, false, true, eye, LC_CK_EYE);
	ChangeKey(1, false, true, target, LC_CK_TARGET);
	ChangeKey(1, false, true, &roll, LC_CK_ROLL);
	ChangeKey(1, true, true, eye, LC_CK_EYE);
	ChangeKey(1, true, true, target, LC_CK_TARGET);
	ChangeKey(1, true, true, &roll, LC_CK_ROLL);

	int i, max = 0;

	for (;;)
	{
		if (strncmp(pCamera->m_Name, "Camera ", 7) == 0)
			if (sscanf(pCamera->m_Name, "Camera %d", &i) == 1)
				if (i > max) 
					max = i;

		if (pCamera->m_Next == NULL)
		{
			char Name[256];
			sprintf(Name, "Camera %d", max+1);
			m_Name = Name;
			pCamera->m_Next = this;
			break;
		}
		else
			pCamera = pCamera->m_Next;
	}

	UpdatePosition(1, false);
}

// From LC_ACTION_CAMERA
lcCamera::lcCamera(float ex, float ey, float ez, float tx, float ty, float tz, lcObject* pCamera)
	: lcObject(LC_OBJECT_CAMERA)
{
	Initialize();

	float eye[3] = { ex, ey, ez }, target[3] = { tx, ty, tz };
	float roll = 0.0f;

	ChangeKey(1, false, true, eye, LC_CK_EYE);
	ChangeKey(1, false, true, target, LC_CK_TARGET);
	ChangeKey(1, false, true, &roll, LC_CK_ROLL);
	ChangeKey(1, true, true, eye, LC_CK_EYE);
	ChangeKey(1, true, true, target, LC_CK_TARGET);
	ChangeKey(1, true, true, &roll, LC_CK_ROLL);

	int i, max = 0;

	if (pCamera)
		for (;;)
		{
			if (strncmp(pCamera->m_Name, "Camera ", 7) == 0)
				if (sscanf(pCamera->m_Name, "Camera %d", &i) == 1)
					if (i > max) 
						max = i;

			if (pCamera->m_Next == NULL)
			{
				char Name[256];
				sprintf(Name, "Camera %d", max+1);
				m_Name = Name;
				pCamera->m_Next = this;
				break;
			}
			else
				pCamera = pCamera->m_Next;
		}

	UpdatePosition(1, false);
}

lcCamera::~lcCamera()
{
	delete m_Target;
}

void lcCamera::Initialize()
{
	m_FOV = 30;
	m_NearDist = 1;
	m_FarDist = 500;

	m_Next = NULL;
	m_nState = 0;
	m_nType = LC_CAMERA_USER;

	m_pTR = NULL;
	m_Name = "";

	float *values[] = { m_Position, m_TargetPosition, &m_Roll };
	RegisterKeys(values, camera_key_info, LC_CK_COUNT);

	m_Target = new CameraTarget(this);
}

/////////////////////////////////////////////////////////////////////////////
// Camera save/load

bool lcCamera::FileLoad(File& file)
{
	unsigned char version, ch;

	file.ReadByte(&version, 1);

	if (version > LC_CAMERA_SAVE_VERSION)
		return false;

	if (version > 5)
		if (!lcObject::FileLoad(file))
			return false;

	if (version == 4)
	{
		char buf[81];
		file.Read(buf, 80);
		buf[80] = 0;
		m_Name = buf;
	}
	else
	{
		file.Read(&ch, 1);
		if (ch == 0xFF)
			return false; // don't read CString
		char buf[81];
		file.Read(buf, ch);
		buf[ch] = 0;
		m_Name = buf;
	}

	if (version < 3)
	{
		double d[3];
		float f[3];

		file.ReadDouble(d, 3);
		f[0] = (float)d[0];
		f[1] = (float)d[1];
		f[2] = (float)d[2];
		ChangeKey(1, false, true, f, LC_CK_EYE);
		ChangeKey(1, true, true, f, LC_CK_EYE);

		file.ReadDouble(d, 3);
		f[0] = (float)d[0];
		f[1] = (float)d[1];
		f[2] = (float)d[2];
		ChangeKey(1, false, true, f, LC_CK_TARGET);
		ChangeKey(1, true, true, f, LC_CK_TARGET);

		file.ReadDouble(d, 3);
		f[0] = (float)d[0];
		f[1] = (float)d[1];
		f[2] = (float)d[2];
//		ChangeKey(1, false, true, f, LC_CK_UP);
//		ChangeKey(1, true, true, f, LC_CK_UP);
		float roll = 0.0f;
		ChangeKey(1, false, true, &roll, LC_CK_ROLL);
		ChangeKey(1, true, true, &roll, LC_CK_ROLL);
	}

	if (version == 3)
	{
		file.Read(&ch, 1);

		while (ch--)
		{
			unsigned char step;
			double eye[3], target[3], up[3];
			float f[3];

			file.ReadDouble(eye, 3);
			file.ReadDouble(target, 3);
			file.ReadDouble(up, 3);
			file.ReadByte(&step, 1);

			if (up[0] == 0 && up[1] == 0 && up[2] == 0)
				up[2] = 1;

			f[0] = (float)eye[0];
			f[1] = (float)eye[1];
			f[2] = (float)eye[2];
			ChangeKey(step, false, true, f, LC_CK_EYE);
			ChangeKey(step, true, true, f, LC_CK_EYE);

			f[0] = (float)target[0];
			f[1] = (float)target[1];
			f[2] = (float)target[2];
			ChangeKey(step, false, true, f, LC_CK_TARGET);
			ChangeKey(step, true, true, f, LC_CK_TARGET);

			f[0] = (float)up[0];
			f[1] = (float)up[1];
			f[2] = (float)up[2];
//			ChangeKey(step, false, true, f, LC_CK_UP);
//			ChangeKey(step, true, true, f, LC_CK_UP);
			float roll = 0.0f;
			ChangeKey(step, false, true, &roll, LC_CK_ROLL);
			ChangeKey(step, true, true, &roll, LC_CK_ROLL);

			int snapshot; // BOOL under Windows
			int cam;
			file.ReadLong(&snapshot, 1);
			file.ReadLong(&cam, 1);
//			if (cam == -1)
//				node->pCam = NULL;
//			else
//				node->pCam = pDoc->GetCamera(i);
		}
	}

	if (version < 4)
	{
		double d;
		file.ReadDouble(&d, 1); m_FOV = (float)d;
		file.ReadDouble(&d, 1); m_FarDist = (float)d;
		file.ReadDouble(&d, 1); m_NearDist = (float)d;
	}
	else
	{
		int n;

		if (version < 6)
		{
			unsigned short time;
			float param[4];
			unsigned char type;

			file.ReadLong(&n, 1);
			while (n--)
			{
				file.ReadShort(&time, 1);
				file.ReadFloat(param, 3);
				file.ReadByte(&type, 1);

				ChangeKey(time, false, true, param, type);
			}

			file.ReadLong(&n, 1);
			while (n--)
			{
				file.ReadShort(&time, 1);
				file.ReadFloat(param, 3);
				file.ReadByte(&type, 1);

				ChangeKey(time, true, true, param, type);
			}
		}

		file.ReadFloat(&m_FOV, 1);
		file.ReadFloat(&m_FarDist, 1);
		file.ReadFloat(&m_NearDist, 1);

		if (version < 5)
		{
			file.ReadLong(&n, 1);
			if (n != 0)
				m_nState |= LC_CAMERA_HIDDEN;
		}
		else
		{
			file.ReadByte(&m_nState, 1);
			file.ReadByte(&m_nType, 1);
		}
	}

	if ((version > 1) && (version < 4))
	{
		unsigned long show;
		int user;

		file.ReadLong(&show, 1);
//		if (version > 2)
		file.ReadLong(&user, 1);
		if (show == 0)
			m_nState |= LC_CAMERA_HIDDEN;
	}

	return true;
}

void lcCamera::FileSave(File& file) const
{
	unsigned char ch = LC_CAMERA_SAVE_VERSION;

	file.WriteByte(&ch, 1);

	lcObject::FileSave(file);

	ch = (unsigned char)m_Name.GetLength();
	file.Write(&ch, 1);
	file.Write((char*)m_Name, ch);

	file.WriteFloat(&m_FOV, 1);
	file.WriteFloat(&m_FarDist, 1);
	file.WriteFloat(&m_NearDist, 1);
	// version 5
	file.WriteByte(&m_nState, 1);
	file.WriteByte(&m_nType, 1);
}

/////////////////////////////////////////////////////////////////////////////
// Camera operations

void lcCamera::Move(unsigned short nTime, bool bAnimation, bool bAddKey, float dx, float dy, float dz)
{
	if (IsSide())
	{
		m_Position += Vector3(dx, dy, dz);
		m_TargetPosition += Vector3(dx, dy, dz);

		ChangeKey(nTime, bAnimation, bAddKey, m_Position, LC_CK_EYE);
		ChangeKey(nTime, bAnimation, bAddKey, m_TargetPosition, LC_CK_TARGET);
	}
	else
	{
		if (IsEyeSelected())
		{
			m_Position += Vector3(dx, dy, dz);

			ChangeKey(nTime, bAnimation, bAddKey, m_Position, LC_CK_EYE);
		}

		if (IsTargetSelected())
		{
			m_TargetPosition += Vector3(dx, dy, dz);

			ChangeKey(nTime, bAnimation, bAddKey, m_TargetPosition, LC_CK_TARGET);
		}
	}
}

void lcCamera::Select(bool bSelecting, bool bFocus, bool bMultiple)
{
	if (bSelecting == true)
	{
		if (bFocus == true)
		{
			m_nState |= (LC_CAMERA_FOCUSED|LC_CAMERA_SELECTED);

			m_Target->Select(false, true, bMultiple);
		}
		else
			m_nState |= LC_CAMERA_SELECTED;

		if (bMultiple == false)
			m_Target->Select(false, false, bMultiple);
	}
	else
	{
		if (bFocus == true)
			m_nState &= ~(LC_CAMERA_FOCUSED);
		else
			m_nState &= ~(LC_CAMERA_SELECTED|LC_CAMERA_FOCUSED);
	} 
}

void lcCamera::SelectTarget(bool bSelecting, bool bFocus, bool bMultiple)
{
	// FIXME: the target should handle this

	if (bSelecting == true)
	{
		if (bFocus == true)
		{
			m_nState |= (LC_CAMERA_TARGET_FOCUSED|LC_CAMERA_TARGET_SELECTED);

			Select(false, true, bMultiple);
		}
		else
			m_nState |= LC_CAMERA_TARGET_SELECTED;

		if (bMultiple == false)
			Select(false, false, bMultiple);
	}
	else
	{
		if (bFocus == true)
			m_nState &= ~(LC_CAMERA_TARGET_FOCUSED);
		else
			m_nState &= ~(LC_CAMERA_TARGET_SELECTED|LC_CAMERA_TARGET_FOCUSED);
	} 
}

void lcCamera::UpdatePosition(u32 Time, bool Animation)
{
	CalculateKeys(Time, Animation);

	Vector3 Z = Normalize(m_Position - m_TargetPosition);

	// Build the Y vector of the matrix.
	Vector3 UpVector;

	if (fabsf(Z[0]) < 0.001f && fabsf(Z[1]) < 0.001f)
		UpVector = Vector3(-Z[2], 0, 0);
	else
		UpVector = Vector3(0, 0, 1);

	// Calculate X vector.
	Vector3 X = Cross(UpVector, Z);

	// Calculate real Y vector.
	Vector3 Y = Cross(Z, X);

	// Apply the roll rotation and recalculate X and Y.
	Matrix33 RollMat = MatrixFromAxisAngle(Z, m_Roll);
	Y = Normalize(Mul(Y, RollMat));
	X = Normalize(Cross(Y, Z));

	// Build matrices.
	Vector4 Row0 = Vector4(X[0], Y[0], Z[0], 0.0f);
	Vector4 Row1 = Vector4(X[1], Y[1], Z[1], 0.0f);
	Vector4 Row2 = Vector4(X[2], Y[2], Z[2], 0.0f);
	Vector4 Row3 = Vector4(Vector3(Row0 * -m_Position[0] + Row1 * -m_Position[1] + Row2 * -m_Position[2]), 1.0f);

	m_WorldView = Matrix44(Row0, Row1, Row2, Row3);
	m_ViewWorld = RotTranInverse(m_WorldView);
}

void lcCamera::Render(float fLineWidth)
{
	// Draw camera.
	glPushMatrix();
	glMultMatrixf(m_ViewWorld);

	glEnableClientState(GL_VERTEX_ARRAY);
	float verts[34][3] =
	{
		{  0.3f,  0.3f,  0.3f }, { -0.3f,  0.3f,  0.3f },
		{ -0.3f,  0.3f,  0.3f }, { -0.3f, -0.3f,  0.3f },
		{ -0.3f, -0.3f,  0.3f }, {  0.3f, -0.3f,  0.3f },
		{  0.3f, -0.3f,  0.3f }, {  0.3f,  0.3f,  0.3f },
		{  0.3f,  0.3f, -0.3f }, { -0.3f,  0.3f, -0.3f },
		{ -0.3f,  0.3f, -0.3f }, { -0.3f, -0.3f, -0.3f },
		{ -0.3f, -0.3f, -0.3f }, {  0.3f, -0.3f, -0.3f },
		{  0.3f, -0.3f, -0.3f }, {  0.3f,  0.3f, -0.3f },
		{  0.3f,  0.3f,  0.3f }, {  0.3f,  0.3f, -0.3f },
		{ -0.3f,  0.3f,  0.3f }, { -0.3f,  0.3f, -0.3f },
		{ -0.3f, -0.3f,  0.3f }, { -0.3f, -0.3f, -0.3f },
		{  0.3f, -0.3f,  0.3f }, {  0.3f, -0.3f, -0.3f },
		{ -0.3f, -0.3f, -0.6f }, { -0.3f,  0.3f, -0.6f },
		{  0.0f,  0.0f, -0.3f }, { -0.3f, -0.3f, -0.6f },
		{  0.3f, -0.3f, -0.6f }, {  0.0f,  0.0f, -0.3f },
		{  0.3f,  0.3f, -0.6f }, {  0.3f, -0.3f, -0.6f },
		{  0.3f,  0.3f, -0.6f }, { -0.3f,  0.3f, -0.6f }
	};

	if (IsEyeSelected())
	{
		glLineWidth(2.0f);
		lcSetColor((m_nState & LC_CAMERA_FOCUSED) != 0 ? LC_COLOR_FOCUS : LC_COLOR_SELECTION);
	}
	else
	{
		glLineWidth(1.0f);
		glColor4f(0.5f, 0.8f, 0.5f, 1.0f);
	}

	glVertexPointer(3, GL_FLOAT, 0, verts);
	glDrawArrays(GL_LINES, 0, 24);
	glDrawArrays(GL_LINE_STRIP, 24, 10);

	glPopMatrix();

	// Draw target box.
	glPushMatrix();
	Matrix44 TargetMat = m_ViewWorld;
	TargetMat.SetTranslation(m_TargetPosition);
	glMultMatrixf(TargetMat);

	if (IsTargetSelected())
	{
		glLineWidth(2.0f);
		lcSetColor((m_nState & LC_CAMERA_TARGET_FOCUSED) != 0 ? LC_COLOR_FOCUS : LC_COLOR_SELECTION);
	}
	else
	{
		glLineWidth(1.0f);
		glColor4f(0.5f, 0.8f, 0.5f, 1.0f);
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	float box[24][3] =
	{ 
		{  0.2f,  0.2f,  0.2f }, { -0.2f,  0.2f,  0.2f },
		{ -0.2f,  0.2f,  0.2f }, { -0.2f, -0.2f,  0.2f },
		{ -0.2f, -0.2f,  0.2f }, {  0.2f, -0.2f,  0.2f },
		{  0.2f, -0.2f,  0.2f }, {  0.2f,  0.2f,  0.2f },
		{  0.2f,  0.2f, -0.2f }, { -0.2f,  0.2f, -0.2f },
		{ -0.2f,  0.2f, -0.2f }, { -0.2f, -0.2f, -0.2f },
		{ -0.2f, -0.2f, -0.2f }, {  0.2f, -0.2f, -0.2f },
		{  0.2f, -0.2f, -0.2f }, {  0.2f,  0.2f, -0.2f },
		{  0.2f,  0.2f,  0.2f }, {  0.2f,  0.2f, -0.2f },
		{ -0.2f,  0.2f,  0.2f }, { -0.2f,  0.2f, -0.2f },
		{ -0.2f, -0.2f,  0.2f }, { -0.2f, -0.2f, -0.2f },
		{  0.2f, -0.2f,  0.2f }, {  0.2f, -0.2f, -0.2f }
	};
	glVertexPointer(3, GL_FLOAT, 0, box);
	glDrawArrays(GL_LINES, 0, 24);
	glPopMatrix();

	glLineWidth(1.0f);

	float Line[2][3] =
	{
		{ m_Position[0], m_Position[1], m_Position[2] },
		{ m_TargetPosition[0], m_TargetPosition[1], m_TargetPosition[2] },
	};

	glVertexPointer(3, GL_FLOAT, 0, Line);
	glColor4f(0.5f, 0.8f, 0.5f, 1.0f);
	glDrawArrays(GL_LINES, 0, 2);

	if (IsSelected())
	{
		glPushMatrix();
		glMultMatrixf(m_ViewWorld);

		float Dist = Length(m_TargetPosition - m_Position);
		Matrix44 Projection;
		Projection = CreatePerspectiveMatrix(m_FOV, 1.33f, 0.01f, Dist);
		Projection = Inverse(Projection);
		glMultMatrixf(Projection);

		// Draw the view frustum.
		float verts[16][3] =
		{
			{  1,  1,  1 }, { -1,  1, 1 },
			{ -1,  1,  1 }, { -1, -1, 1 },
			{ -1, -1,  1 }, {  1, -1, 1 },
			{  1, -1,  1 }, {  1,  1, 1 },
			{  1,  1, -1 }, {  1,  1, 1 },
			{ -1,  1, -1 }, { -1,  1, 1 },
			{ -1, -1, -1 }, { -1, -1, 1 },
			{  1, -1, -1 }, {  1, -1, 1 },
		};

		glVertexPointer(3, GL_FLOAT, 0, verts);
		glDrawArrays(GL_LINES, 0, 16);

		glPopMatrix();
	}

	glDisableClientState(GL_VERTEX_ARRAY);
}

void lcCamera::ClosestLineIntersect(lcClickLine& ClickLine) const
{
	BoundingBox Box;
	Box.m_Max = Vector3(0.2f, 0.2f, 0.2f);
	Box.m_Min = Vector3(-0.2f, -0.2f, -0.2f);

	Vector3 Start = Mul31(ClickLine.Start, m_WorldView);
	Vector3 End = Mul31(ClickLine.End, m_WorldView);

	float Dist;
	if (BoundingBoxRayMinIntersectDistance(Box, Start, End, &Dist) && (Dist < ClickLine.Dist))
	{
		ClickLine.Object = this;
		ClickLine.Dist = Dist;
	}

	m_Target->ClosestLineIntersect(ClickLine);
}

bool lcCamera::IntersectsVolume(const Vector4* Planes, int NumPlanes) const
{
	BoundingBox Box;
	Box.m_Max = Vector3(0.3f, 0.3f, 0.3f);
	Box.m_Min = Vector3(-0.3f, -0.3f, -0.3f);

	// Transform the planes to local space.
	Vector4* LocalPlanes = new Vector4[NumPlanes];
	int i;

	for (i = 0; i < NumPlanes; i++)
	{
		LocalPlanes[i] = Vector4(Mul30(Vector3(Planes[i]), m_WorldView));
		LocalPlanes[i][3] = Planes[i][3] - Dot3(Vector3(m_WorldView[3]), Vector3(LocalPlanes[i]));
	}

	bool Intersect = BoundingBoxIntersectsVolume(Box, LocalPlanes, NumPlanes);

	delete[] LocalPlanes;

	if (!Intersect)
		Intersect = m_Target->IntersectsVolume(Planes, NumPlanes);

	return Intersect;
}

void lcCamera::LoadProjection(float fAspect)
{
	if (m_pTR != NULL)
		m_pTR->BeginTile();
	else
	{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(m_FOV, fAspect, m_NearDist, m_FarDist);
	}

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(m_WorldView);
}

void lcCamera::Zoom(u32 Time, bool Animation, bool AddKey, int MouseX, int MouseY)
{
	float Sensitivity = 2.0f / (LC_MAX_MOUSE_SENSITIVITY+1 - g_App->m_MouseSensitivity);
	float dy = MouseY * Sensitivity;

	if (IsOrtho())
	{
		// TODO: have a different option to change the FOV.
		m_FOV += dy;
		m_FOV = lcClamp(m_FOV, 0.001f, 179.999f);
	}
	else
	{
		Vector3 Delta = Vector3(m_ViewWorld[2]) * dy;

		// TODO: option to move eye, target or both
		m_Position += Delta;
		m_TargetPosition += Delta;

		ChangeKey(Time, Animation, AddKey, m_Position, LC_CK_EYE);
		ChangeKey(Time, Animation, AddKey, m_TargetPosition, LC_CK_TARGET);
	}

	UpdatePosition(Time, Animation);
}

void lcCamera::Pan(u32 Time, bool Animation, bool AddKey, int MouseX, int MouseY)
{
	float Sensitivity = 2.0f / (LC_MAX_MOUSE_SENSITIVITY+1 - g_App->m_MouseSensitivity);
	float dx = MouseX * Sensitivity;
	float dy = MouseY * Sensitivity;

	Vector3 Delta = Vector3(m_ViewWorld[0]) * -dx + Vector3(m_ViewWorld[1]) * -dy;

	m_Position += Delta;
	m_TargetPosition += Delta;

	ChangeKey(Time, Animation, AddKey, m_Position, LC_CK_EYE);
	ChangeKey(Time, Animation, AddKey, m_TargetPosition, LC_CK_TARGET);
	UpdatePosition(Time, Animation);
}

void lcCamera::Orbit(u32 Time, bool Animation, bool AddKey, int MouseX, int MouseY)
{
	float Sensitivity = 2.0f / (LC_MAX_MOUSE_SENSITIVITY+1 - g_App->m_MouseSensitivity);
	float dx = MouseX * Sensitivity;
	float dy = MouseY * Sensitivity;

	Vector3 Dir = m_Position - m_TargetPosition;

	// The X axis of the mouse always corresponds to Z in the world.
	if (fabsf(dx) > 0.01f)
	{
		float AngleX = -dx * LC_DTOR;
		Matrix33 RotX = MatrixFromAxisAngle(Vector4(0, 0, 1, AngleX));

		Dir = Mul(Dir, RotX);
	}

	// The Y axis will the side vector of the camera.
	if (fabsf(dy) > 0.01f)
	{
		float AngleY = dy * LC_DTOR;
		Matrix33 RotY = MatrixFromAxisAngle(Vector4(m_WorldView[0][0], m_WorldView[1][0], m_WorldView[2][0], AngleY));

		Dir = Mul(Dir, RotY);
	}

	ChangeKey(Time, Animation, AddKey, Dir + m_TargetPosition, LC_CK_EYE);
	UpdatePosition(Time, Animation);
}

void lcCamera::Rotate(u32 Time, bool Animation, bool AddKey, int MouseX, int MouseY)
{
	float Sensitivity = 2.0f / (LC_MAX_MOUSE_SENSITIVITY+1 - g_App->m_MouseSensitivity);
	float dx = MouseX * Sensitivity;
	float dy = MouseY * Sensitivity;

	Vector3 Dir = m_TargetPosition - m_Position;

	// The X axis of the mouse always corresponds to Z in the world.
	if (fabsf(dx) > 0.01f)
	{
		float AngleX = -dx * LC_DTOR;
		Matrix33 RotX = MatrixFromAxisAngle(Vector4(0, 0, 1, AngleX));

		Dir = Mul(Dir, RotX);
	}

	// The Y axis will the side vector of the camera.
	if (fabsf(dy) > 0.01f)
	{
		float AngleY = dy * LC_DTOR;
		Matrix33 RotY = MatrixFromAxisAngle(Vector4(m_WorldView[0][0], m_WorldView[1][0], m_WorldView[2][0], AngleY));

		Dir = Mul(Dir, RotY);
	}

	ChangeKey(Time, Animation, AddKey, Dir + m_Position, LC_CK_TARGET);
	UpdatePosition(Time, Animation);
}

void lcCamera::Roll(u32 Time, bool Animation, bool AddKey, int MouseX, int MouseY)
{
	float Sensitivity = 2.0f / (LC_MAX_MOUSE_SENSITIVITY+1 - g_App->m_MouseSensitivity);
	float dx = MouseX * Sensitivity;

	float NewRoll = m_Roll + dx / 100;

	ChangeKey(Time, Animation, AddKey, &NewRoll, LC_CK_ROLL);
	UpdatePosition(Time, Animation);
}

void lcCamera::StartTiledRendering(int tw, int th, int iw, int ih, float fAspect)
{
	m_pTR = new TiledRender();
	m_pTR->TileSize(tw, th, 0);
	m_pTR->ImageSize(iw, ih);
	m_pTR->Perspective(m_FOV, fAspect, m_NearDist, m_FarDist);
}

void lcCamera::GetTileInfo(int* row, int* col, int* width, int* height)
{
	if (m_pTR != NULL)
	{
		*row = m_pTR->m_Rows - m_pTR->m_CurrentRow - 1;
		*col = m_pTR->m_CurrentColumn;
		*width = m_pTR->m_CurrentTileWidth;
		*height = m_pTR->m_CurrentTileHeight;
	}
}

bool lcCamera::EndTile()
{
	if (m_pTR != NULL)
	{
		if (m_pTR->EndTile())
			return true;

		delete m_pTR;
		m_pTR = NULL;
	}

	return false;
}
