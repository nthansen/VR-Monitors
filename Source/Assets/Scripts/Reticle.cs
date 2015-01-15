using UnityEngine;
using System.Collections;

public class Reticle : MonoBehaviour {

    public Camera CameraFacing;
    public GameObject selectedMonitor;
    public static Reticle instance;
    public bool moveTheMonitor;

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
            print("I'm looking at " + hit.transform.name);
            // grabs the monitor that's being hit
            selectedMonitor = hit.transform.gameObject;
        }
        else
            print("I'm looking at nothing!");
    }


    // Moves the monitor relative to where the camera is facing
    void moveMonitor()
    {
        Ray ray = CameraFacing.ViewportPointToRay(new Vector3(0.5f, 0.5f, 0f));
        selectedMonitor.transform.position = new Vector3(ray.direction.x * 1000, ray.direction.y * 1000, selectedMonitor.transform.position.z);
    }
}
