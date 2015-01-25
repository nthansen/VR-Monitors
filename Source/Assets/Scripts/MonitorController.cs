using UnityEngine;
using System.Collections;

public class MonitorController : MonoBehaviour {


    public int monitorCount;
    public bool addMonitor;
    public static MonitorController instance;

	// Use this for initialization
    void Start()
    {
        // a way to access this object
        instance = this;

        // added one monitor
        monitorCount = 1;

        GameObject primaryMonitor = GameObject.FindGameObjectWithTag("Monitor");
        primaryMonitor.AddComponent("monitor");

        // the primary monitor is already in the scene so we don't need to add one
        addMonitor = false;

    }
	
	// Update is called once per frame
	void Update () {

        if (addMonitor)
        {
            GameObject[] allMonitors = GameObject.FindGameObjectsWithTag("Monitor");

            GameObject previousMonitor = allMonitors[allMonitors.Length - 1];
            Vector3 previousMonitorPosition = previousMonitor.transform.position;
            Vector3 previousMonitorRotation = previousMonitor.transform.rotation.eulerAngles;

            // add horizontal
            if (monitorCount % 2 == 1)
            {
                // move cube to the left
                previousMonitor.transform.position = new Vector3(previousMonitorPosition.x - 5f, previousMonitorPosition.y, previousMonitorPosition.z);
                previousMonitor.transform.localEulerAngles = new Vector3(previousMonitorRotation.x, -40f, previousMonitorRotation.z);

                // add and move cube to the right
                monitorCount += 1;

            }    
            // add vertical
            else
            {
                // add cube on top of last created cube and in the center

                monitorCount += 1;

            }
            
            addMonitor = false;
        }
	}
}
