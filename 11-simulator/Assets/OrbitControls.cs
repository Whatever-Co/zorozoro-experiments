using UnityEngine;
using UnityEngine.EventSystems;
using System.Collections;

///<summary>
/// メインカメラにアタッチして使用。
///</summary>

[AddComponentMenu("Camera Controls/Orbit Controls")]
public class OrbitControls : MonoBehaviour
{
	[Header("カメラの注視点")]
	public Transform target;

	[Header("カメラ回転速度")]
	public float xSpeed = 20;
	public float ySpeed = 20;

	[Header("カメラズーム速度")]
	public float zoomSpeed = 1;

	[Header("縦回転のリミット")]
	public float yMinLimit = 0;
	public float yMaxLimit = 90;

	[Header("最小・最大距離")]
	public float distanceMin = 1;
	public float distanceMax = 30;

	[Header("マウス操作有効")]
	public bool isMouseControl = true;

	[Header("キー操作有効")]
	public bool isKeyControl = true;


	private float _distance;
	private float _rx = 0;
	private float _ry = 0;

	private float _px = 0;
	private float _py = 0;

	// キー操作など用
	private bool _isRotX = false;
	private float _directionX = 1;
	private bool _isRotY = false;
	private float _directionY = 1;
	private bool _isZoom = false;
	private float _zoomAccel = .1f;
	private bool _isTargetMove;
	private float _targetMoveDirection = 1;

	// Getter/Setter
	public bool isRotX
	{
		get { return this._isRotX; }
		set { this._isRotX = value; }
	}
	public float directionX
	{
		get { return this._directionX; }
		set { this._directionX = value; }
	}
	public bool isRotY
	{
		get { return this._isRotY; }
		set { _isRotY = value; }
	}
	public float directionY
	{
		get { return this._directionY; }
		set { this._directionY = value; }
	}
	public bool isZoom
	{
		get { return this._isZoom; }
		set { _isZoom = value; }
	}
	public float zoomAccel
	{
		get { return this._zoomAccel; }
		set { this._zoomAccel = value; }
	}
	public bool isTargetMove
	{
		get { return this._isTargetMove; }
		set { _isTargetMove = value; }
	}
	public float targetMoveDirection
	{
		get { return this._targetMoveDirection; }
		set { this._targetMoveDirection = value; }
	}



	// 外部のコントローラー（ゲームパッドなど）で操作されているか
	[HideInInspector]
	[System.NonSerialized]
	public bool useOtherController = false;


	private bool _isStart = false;


	void Awake()
	{
		transform.LookAt(Vector3.zero);
	}


	void Start()
	{

		Vector3 angles = transform.eulerAngles;
		_rx = angles.y;
		_ry = angles.x;

		if (target == null)
		{
			target = new GameObject("Look Target").transform;
		}

		_distance = Vector3.Distance(transform.position, target.position);
	}


	void Update()
	{
		// UI などが操作されている場合は処理しない
		if (EventSystem.current.IsPointerOverGameObject())
		{
			return;
		}
		
		// 左右回転（A・d）
		if (Input.GetKey("a"))
		{
			_isRotX = true;
			_directionX = 1;
		}
		else if (Input.GetKey("d"))
		{
			_isRotX = true;
			_directionX = -1;
		}
		else
		{
			if (!useOtherController)
			{
				_isRotX = false;
			}
		}

		// 上下回転（W・S）
		if (Input.GetKey("w"))
		{
			_isRotY = true;
			_directionY = 1;
		}
		else if (Input.GetKey("s"))
		{
			_isRotY = true;
			_directionY = -1;
		}
		else
		{
			if (!useOtherController)
			{
				_isRotY = false;
			}
		}

		// ズーム
		if (Input.GetKey("up"))
		{
			_isZoom = true;
			_zoomAccel = .25f;
		}
		else if (Input.GetKey("down"))
		{
			_isZoom = true;
			_zoomAccel = -.25f;
		}
		else
		{
			if (!useOtherController)
			{
				_isZoom = false;
			}
		}

		// ターゲット移動
		if (Input.GetKey("left"))
		{
			_isTargetMove = true;
			_targetMoveDirection = -1;
		}
		else if (Input.GetKey("right"))
		{
			_isTargetMove = true;
			_targetMoveDirection = 1;
		}
		else
		{
			if (!useOtherController)
			{
				_isTargetMove = false;
			}
		}


	}


	void LateUpdate()
	{
		// UI などが操作されている場合は処理しない
		if (EventSystem.current.IsPointerOverGameObject())
		{
			return;
		}
		
		
		if (target)
		{
			float dx = -(Input.mousePosition.x - _px) * .1f;
			_px = Input.mousePosition.x;

			float dy = -(Input.mousePosition.y - _py) * .1f;
			_py = Input.mousePosition.y;


			// マウス操作用
			if ((Input.GetMouseButton(0) && isMouseControl) || !_isStart)
			{
				//
				_isStart = true;

				//
				_rx += Input.GetAxis("Mouse X") * xSpeed * _distance * 0.02f;
				_ry -= Input.GetAxis("Mouse Y") * ySpeed * 0.25f;
			}
			else if (Input.GetMouseButton(1) && isMouseControl)
			{
				moveForLookDirection(dx, dy, target);
			}

			// キーダウン・仮想コントローラ用
			if (_isRotX)
			{
				_rx += xSpeed * _distance * 0.001f * _directionX;
			}
			if (_isRotY)
			{
				_ry += ySpeed * 0.02f * _directionY;
			}
			if (_isZoom)
			{
				_distance = Mathf.Clamp(_distance - (_zoomAccel * zoomSpeed), distanceMin, distanceMax);
			}
			if (_isTargetMove)
			{
				moveForLookDirection(_targetMoveDirection * .05f, 0, target);
			}

			// マウスホイール用
			if (isMouseControl)
			{
				_distance = Mathf.Clamp(_distance - Input.GetAxis("Mouse ScrollWheel") * (5 * zoomSpeed), distanceMin, distanceMax);
			}

			_ry = ClampAngle(_ry, yMinLimit, yMaxLimit);
			Quaternion rotation = Quaternion.Euler(_ry, _rx, 0);

			Vector3 negDistance = new Vector3(0.0f, 0.0f, -_distance);
			Vector3 position = rotation * negDistance + target.position;

			transform.rotation = rotation;
			transform.position = position;
		}

	}

	private void moveForLookDirection(float dx, float dy, Transform target_tf)
	{
		// カメラの方向から、X-Z平面の単位ベクトルを取得
		Vector3 cameraForward = Vector3.Scale(Camera.main.transform.forward, new Vector3(1, 0, 1)).normalized;
		// 移動ベクトル作成
		Vector3 direction_v = cameraForward * dy + Camera.main.transform.right * dx;

		Debug.Log(direction_v);
		// 移動
		target_tf.Translate(direction_v);
	}


	public static float ClampAngle(float angle, float min, float max)
	{

		if (angle < -360)
		{
			angle += 360;
		}

		if (angle > 360)
		{
			angle -= 360;
		}

		return Mathf.Clamp(angle, min, max);
	}
}