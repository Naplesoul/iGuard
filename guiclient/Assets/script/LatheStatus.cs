using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class LatheStatus : MonoBehaviour
{
    
    public GameObject lathe;
    public GameObject lathe_big_carriage;
    public GameObject lathe_mid_carriage;
    public GameObject lathe_big_wheel;
    public GameObject lathe_mid_wheel;
    public GameObject lathe_axle;


    public static float carriage_x = 0f;
    public static float carriage_z = 0f;
    public static bool running = false;

    private int alert_id;

    // Start is called before the first frame update
    void Start()
    {
        alert_id = -1;
    }

    // Update is called once per frame
    void Update()
    {
        //Debug.Log(carriage_x);
        Vector3 pos = lathe_big_carriage.transform.localPosition;
        pos.x = carriage_x * 0.54f;
        lathe_big_carriage.transform.localPosition = pos;

        Vector3 pos2 = lathe_mid_carriage.transform.localPosition;
        pos2.y = - carriage_z * 0.19f / 8.3731f;
        lathe_mid_carriage.transform.localPosition = pos2;

        Vector3 rot = lathe_big_wheel.transform.localRotation.eulerAngles;
        rot.y = - pos.x * 5000;
        lathe_big_wheel.transform.localRotation = Quaternion.Euler(rot);

        rot = lathe_mid_wheel.transform.localRotation.eulerAngles;
        rot.y = pos2.y * 40000;
        lathe_mid_wheel.transform.localRotation = Quaternion.Euler(rot);

        if (running){
            lathe_axle.transform.Rotate(new Vector3(0, 0, -2.5f));
            if (pos.x > 0.54f){
                alert_id = Alert.updateAlertMsg(alert_id, "即将撞车，请立即停止进刀", 100);
            }else if (alert_id >= 0){
                Alert.removeAlertMsg(alert_id);
                alert_id = -1;
            }
        }
    }
}
