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

    }
}
