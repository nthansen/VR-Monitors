using UnityEngine;
using System.Collections;

public class Reticle : MonoBehaviour {

    public Camera CameraFacing;
	
	// Update is called once per frame
	void Update () {
        // look at where the camera is facing so we can always see the plain
        transform.LookAt(CameraFacing.transform.position);

        // move the position to where the camera is facing

        transform.position = CameraFacing.transform.position +
                    CameraFacing.transform.rotation * Vector3.forward * 2.0f;
        
        // rotate so we can always see the plain since it's 2d
        transform.Rotate(0.0f, 180.0f, 0.0f);

        reticleHit();
    }

    // Used to see if the reticle is pointing at a monitor
    void reticleHit()
    {
        Ray ray = CameraFacing.ViewportPointToRay(new Vector3(0.5f,0.5f,0f));
		// Do a raycast
		RaycastHit hit;
		if (Physics.Raycast(ray, out hit))
			print ("I'm looking at " + hit.transform.name);
		else
			print ("I'm looking at nothing!");
    }
}
