using UnityEngine;
using System.Collections;
using System.Windows.Forms;

public class Reticle : MonoBehaviour {

    public Camera CameraFacing;
    public GameObject selectedMonitor;
    public static Reticle instance;
    public bool moveTheMonitor;
    public bool rotateMonitor;
    float monitorDistance;
    TrackBar resizeMonitorSlider;
    TrackBar rotateMonitorSlider;

    void Awake()
    {
        renderer.enabled = false;
        instance = this;
        moveTheMonitor = false;

    }

	// Update is called once per frame
	void Update () {
        // look at where the camera is facing so we can always see the plain
        transform.LookAt(CameraFacing.transform.position);

        // move the position to where the camera is facing

        transform.position = CameraFacing.transform.position +
                    CameraFacing.transform.rotation * Vector3.forward * 2.0f;
        
        // rotate so we can always see the plain since it's 2d
        transform.Rotate(0.0f, 180.0f, 0.0f);

        if (moveTheMonitor)
        {
            moveMonitor();
        }
        else
        {
            reticleHit();
        }
    }

    // Used to see if the reticle is pointing at a monitor
    void reticleHit()
    {
        Ray ray = CameraFacing.ViewportPointToRay(new Vector3(0.5f,0.5f,0f));
        
		// Do a raycast
		RaycastHit hit;
        if (Physics.Raycast(ray, out hit))
        {

            resizeMonitorSlider = ExternalForm.instance.resizeMonitorSlider;
            rotateMonitorSlider = ExternalForm.instance.rotateMonitorSlider;

            // grabs the monitor that's being hit
            selectedMonitor = hit.transform.gameObject;
            monitorDistance = hit.distance;

            monitor values = (monitor)selectedMonitor.GetComponent("monitor");

            resizeMonitorSlider.Enabled = true;
            resizeMonitorSlider.Value = values.monitorSize;

            rotateMonitorSlider.Enabled = true;

            //values.originalRotation = selectedMonitor.transform.localEulerAngles;

            rotateMonitorSlider.Value = values.monitorRotation;

            //print("I'm looking at Monitor ");

        }
        else
        {
            selectedMonitor = null;
            resizeMonitorSlider.Enabled = false;
            rotateMonitorSlider.Enabled = false;
            //rotateMonitorSlider.Value = 0;
            //print("I'm looking at nothing!");
        }
    }


    // Moves the monitor relative to where the camera is facing
    void moveMonitor()
    {
        if (selectedMonitor != null)
        {
            selectedMonitor.transform.LookAt(CameraFacing.transform.position);

            // move the position to where the camera is facing

            selectedMonitor.transform.position = CameraFacing.transform.position +
                   CameraFacing.transform.rotation * Vector3.forward * monitorDistance;

            selectedMonitor.transform.Rotate(0.0f, 180.0f, 0.0f);
        }
    }
}
