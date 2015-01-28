using UnityEngine;
using System.Collections;

public class MonitorController : MonoBehaviour {


    public int monitorCount;
    public bool addMonitor;
    public static MonitorController instance;

    float monitorWidth;
    float monitorHeight;

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

        monitorWidth = 4.5f;
        monitorHeight = 5.3f;

    }
	
	// Update is called once per frame
	void Update () {

        if (addMonitor)
        {
            monitorCount += 1;

            GameObject[] allMonitors = GameObject.FindGameObjectsWithTag("Monitor");

            GameObject newMonitor = GameObject.CreatePrimitive(PrimitiveType.Quad);
            newMonitor.AddComponent("monitor");
            newMonitor.tag = ("Monitor");

            GameObject previousMonitor = allMonitors[allMonitors.Length - 1];
            Vector3 previousMonitorPosition = previousMonitor.transform.position;
            Vector3 previousMonitorRotation = previousMonitor.transform.rotation.eulerAngles;

            // add horizontal
            if (monitorCount % 2 == 0)
            {
                // move cube to the left
                previousMonitor.transform.position = new Vector3(previousMonitorPosition.x - monitorWidth, previousMonitorPosition.y, previousMonitorPosition.z);
                previousMonitor.transform.localEulerAngles = new Vector3(previousMonitorRotation.x, -20f, previousMonitorRotation.z);

                newMonitor.transform.position = new Vector3(previousMonitorPosition.x + monitorWidth, previousMonitorPosition.y, previousMonitorPosition.z);
                newMonitor.transform.localEulerAngles = new Vector3(previousMonitorRotation.x, 20f, previousMonitorRotation.z);

                // add and move cube to the right
                monitor previousValues = (monitor)previousMonitor.GetComponent("monitor");
                previousValues.monitorRotation = -5;
                monitor newValues = (monitor)newMonitor.GetComponent("monitor");
                newValues.monitorRotation = 5;

            }    
            // add vertical
            else
            {
                // add cube on top of last created cube and in the center

                newMonitor.transform.position = new Vector3(0, previousMonitorPosition.y + monitorHeight, previousMonitorPosition.z);
                newMonitor.transform.localEulerAngles = Vector3.zero;

            }


            newMonitor = null;
            previousMonitor = null;
            addMonitor = false;
        }
	}

    public void resetMonitors()
    {

        GameObject[] allMonitors = GameObject.FindGameObjectsWithTag("Monitor");

        monitor values;

        allMonitors[0].transform.position = new Vector3(0, 0, 5);
        allMonitors[0].transform.localEulerAngles = Vector3.zero;
        values = (monitor)allMonitors[0].GetComponent("monitor");
        allMonitors[0].transform.localScale = values.originalScale;
        values.monitorRotation = 0;
        values.monitorSize = 0;
        

        for (int i = 1; i < monitorCount; i++)
        {
            if (i % 2 == 0)
            {
                allMonitors[i].transform.position = new Vector3(0, allMonitors[i - 1].transform.position.y + monitorHeight, 5);
                allMonitors[i].transform.localEulerAngles = Vector3.zero;
                values = (monitor)allMonitors[i].GetComponent("monitor");
                allMonitors[i].transform.localScale = values.originalScale;
                values.monitorRotation = 0;
                values.monitorSize = 0;
            }
            else
            {
                Vector3 previousMonitorPosition = allMonitors[i - 1].transform.position;
                Vector3 previousMonitorRotation = allMonitors[i - 1].transform.position;

                allMonitors[i - 1].transform.position = new Vector3(previousMonitorPosition.x - monitorWidth, previousMonitorPosition.y, previousMonitorPosition.z);
                allMonitors[i - 1].transform.localEulerAngles = new Vector3(0, -20f, 0);

                allMonitors[i].transform.position = new Vector3(previousMonitorPosition.x + monitorWidth, previousMonitorPosition.y, previousMonitorPosition.z);
                allMonitors[i].transform.localEulerAngles = new Vector3(0, 20f, 0);

                // add and move cube to the right
                monitor previousValues = (monitor)allMonitors[i - 1].GetComponent("monitor");
                previousValues.monitorRotation = -5;
                monitor newValues = (monitor)allMonitors[i].GetComponent("monitor");
                newValues.monitorRotation = 5;
            }
        }
    }
}
