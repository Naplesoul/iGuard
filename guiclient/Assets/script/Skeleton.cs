using System.Collections;
using System.Collections.Generic;
using System.Net.Sockets;
using System.Net;
using System.Text;
using System.Threading;
using UnityEngine;
using UnityEngine.UI;
using System.IO;
using System;

[System.Serializable]
public class KeyPack_Vector3{
    public float x;
    public float y;
    public float z;
}

[System.Serializable]
public class KeyPack_NodeInfo{
    public string type;
    public string pose;
    public bool running;
    public float carriage_x_high;
    public float carriage_x_low;
    public string motion;
    public string motion_name;
    public float motion_value;
    public KeyPack_Vector3[] nodes;
}


[System.Serializable]
public class WebPack_Vector3{
    public float x;
    public float y;
    public float z;
    public float score;
}
[System.Serializable]
public class WebPack_Body_Metrics{
    public float arm_width;
    public float head_width;
    public float leg_width;
    public float torso_width;
}

[System.Serializable]
public class WebPack_NodeInfo{
    public long frame_id;
    public float carriage_x;
    public float carriage_z;
    public bool running;

    public WebPack_Body_Metrics body_metrics;
    
    public WebPack_Vector3[] body_nodes;
    public WebPack_Vector3[] left_hand_nodes;
    public WebPack_Vector3[] right_hand_nodes;

}
public class Skeleton : MonoBehaviour
{
    private static string ip = "0.0.0.0";
    private static int port = 50002;
    private static Socket socket;
    static public StreamWriter log_file;

    public static float sim;

    private GameObject[] body_nodes;
    private GameObject[] body_bones;
    public GameObject head, shouder, body, hip, waist;
    private WebPack_Body_Metrics bm = new WebPack_Body_Metrics();

    private GameObject[] lh_nodes;
    private GameObject[] lh_bones;
    private GameObject[] rh_nodes;
    private GameObject[] rh_bones;


    private Vector2Int[] bnMap, hbnMap;
    private Vector3[] body_nodePos, body_lastNodePos;
    private Vector3[] lh_nodePos, lh_lastNodePos;
    private Vector3[] rh_nodePos, rh_lastNodePos;

    private bool[] body_nodeActive, body_boneActive;
    private bool[] lh_nodeActive, lh_boneActive;
    private bool[] rh_nodeActive, rh_boneActive;
    public GameObject nodePrefab;
    public GameObject pPrefab;
    public GameObject bonePrefab;
    private Vector3[] keyPose, okPose;

    private int keyAlertId = -1;
    private int ppe_helmetAlertId = -2;
    private int ppe_gloveAlertId = -2;
    private int ppe_goggleAlertId = -2;

    private GameObject[] preNodes;
    private Vector3[] prePose;

    private string[] keyPoseNameList;
    private KeyPack_NodeInfo[] keyPoseList;
    private string[] actionList;
    private int currKeyPoseIndex, selectIndex;
    private long last_frame_id;
    private float last_ok_time;

    private Vector3 camera_relative_pos;

    public Material keyMat;
    public Material defMat;
    public Material preMat;
    public Material dangerMat;
    public Text dangerText;
    public Text simiText;
    public Text nextText;
    public Text timeText;
    public Text actText;
    public Text simText;
    public GameObject lathe;
    public GameObject driller;
    private GameObject currentObject;
    public GameObject[] dangerAreas;
    private bool machine_running = false;

    public GameObject helmet;
    public GameObject goggle;
    public GameObject glove_left;
    public GameObject glove_right;

    public TMPro.TMP_InputField input_text;
    public GameObject edit_panel;
    public Button btnPrefab;

    public bool ext;


    // Start is called before the first frame update
    void Start()
    {
        Screen.SetResolution(2560, 1440, true, 60);
        log_file = new StreamWriter("./log.md");

        body_nodes = new GameObject[19];
        lh_nodes = new GameObject[21];
        rh_nodes = new GameObject[21];
        body_bones = new GameObject[18];
        lh_bones = new GameObject[21];
        rh_bones = new GameObject[21];
        body_nodePos = new Vector3[19];
        body_lastNodePos = new Vector3[19];
        lh_nodePos = new Vector3[21];
        rh_nodePos = new Vector3[21];
        lh_lastNodePos = new Vector3[21];
        rh_lastNodePos = new Vector3[21];

        body_nodeActive = new bool[19];
        body_boneActive = new bool[18];
        lh_nodeActive = new bool[21];
        lh_boneActive = new bool[21];
        rh_nodeActive = new bool[21];
        rh_boneActive = new bool[21];

        bnMap = new Vector2Int[18];
        hbnMap = new Vector2Int[21];
        keyPose = new Vector3[19];
        okPose = new Vector3[21];
        prePose = new Vector3[19];
        preNodes = new GameObject[19];
        currKeyPoseIndex = 0;
        last_frame_id = -1;

        camera_relative_pos = new Vector3(1.8f, -1.2f, 0.8f);
        
        for (int i = 0; i < 19 ;i ++){
            body_nodes[i] = GameObject.Instantiate(nodePrefab, new Vector3(0, -10, 0), new Quaternion());
            body_nodes[i].name = "node-" + i;
            preNodes[i] = GameObject.Instantiate(pPrefab, new Vector3(0, -10, 0), new Quaternion());
            preNodes[i].name = "pNode-" + i;
        }
        for (int i = 0; i < 18 ;i ++){
            
            //test new bones
            // if (i == 2){
            //     body_bones[i] = GameObject.Find("body4-2");
            // }else if (i == 3){
            //     body_bones[i] = GameObject.Find("body2-3");
            // }else if (i == 0){
            //     body_bones[i] = GameObject.Find("body0-1");
            // }

            body_bones[i] = GameObject.Instantiate(bonePrefab, new Vector3(0, -10, 0), new Quaternion());
            body_bones[i].name = "bone-" + i;
        }
        for (int i = 0; i < 21 ;i ++){
            lh_nodes[i] = GameObject.Instantiate(nodePrefab, new Vector3(0, -10, 0), new Quaternion());
            lh_nodes[i].name = "lhnode-" + i;
            lh_nodes[i].transform.localScale = new Vector3(0.01f, 0.01f, 0.01f); 
        }
        for (int i = 0; i < 21 ;i ++){
            rh_nodes[i] = GameObject.Instantiate(nodePrefab, new Vector3(0, -10, 0), new Quaternion());
            rh_nodes[i].name = "rhnode-" + i;
            rh_nodes[i].transform.localScale = new Vector3(0.01f, 0.01f, 0.01f); 
        }
        for (int i = 0; i < 21 ;i ++){
            lh_bones[i] = GameObject.Instantiate(bonePrefab, new Vector3(0, -10, 0), new Quaternion());
            lh_bones[i].name = "bone-" + i;
        }
        for (int i = 0; i < 21 ;i ++){
            rh_bones[i] = GameObject.Instantiate(bonePrefab, new Vector3(0, -10, 0), new Quaternion());
            rh_bones[i].name = "bone-" + i;
        }
        bnMap[0] = new Vector2Int(0, 1);
        bnMap[1] = new Vector2Int(1, 4);
        bnMap[2] = new Vector2Int(4, 2);
        bnMap[3] = new Vector2Int(2, 3);
        bnMap[4] = new Vector2Int(4, 5);
        bnMap[5] = new Vector2Int(5, 6);
        bnMap[6] = new Vector2Int(6, 7);
        bnMap[7] = new Vector2Int(7, 8);
        bnMap[8] = new Vector2Int(4, 9);
        bnMap[9] = new Vector2Int(9, 10);
        bnMap[10] = new Vector2Int(10, 11);
        bnMap[11] = new Vector2Int(11, 12);
        bnMap[12] = new Vector2Int(3, 13);
        bnMap[13] = new Vector2Int(13, 14);
        bnMap[14] = new Vector2Int(14, 15);
        bnMap[15] = new Vector2Int(3, 16);
        bnMap[16] = new Vector2Int(16, 17);
        bnMap[17] = new Vector2Int(17, 18);

        hbnMap[0] = new Vector2Int(0, 1);
        hbnMap[1] = new Vector2Int(1, 2);
        hbnMap[2] = new Vector2Int(2, 3);
        hbnMap[3] = new Vector2Int(3, 4);
        hbnMap[4] = new Vector2Int(0, 5);
        hbnMap[5] = new Vector2Int(5, 6);
        hbnMap[6] = new Vector2Int(6, 7);
        hbnMap[7] = new Vector2Int(7, 8);
        hbnMap[8] = new Vector2Int(5, 9);
        hbnMap[9] = new Vector2Int(9, 10);
        hbnMap[10] = new Vector2Int(10, 11);
        hbnMap[11] = new Vector2Int(11, 12);
        hbnMap[12] = new Vector2Int(9, 13);
        hbnMap[13] = new Vector2Int(13, 14);
        hbnMap[14] = new Vector2Int(14, 15);
        hbnMap[15] = new Vector2Int(15, 16);
        hbnMap[16] = new Vector2Int(13, 17);
        hbnMap[17] = new Vector2Int(17, 18);
        hbnMap[18] = new Vector2Int(18, 19);
        hbnMap[19] = new Vector2Int(19, 20);
        hbnMap[20] = new Vector2Int(0, 17);

        DangerCol.dangerText = dangerText;
        dangerText.text = "当前无危险";
        DangerCol.socket = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
        CaseChanged(0);
        LoadOKPos();

        
        socket = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
        socket.Bind(new IPEndPoint(IPAddress.Parse(ip), port));
        Thread threadReceive = new Thread(new ThreadStart(receiveFromClient));
        threadReceive.Start();

        Debug.Log("skeleton server initialized successfully!");
        LogLine("**iGuard Start**");
    }

    // Update is called once per frame
    void Update()
    {
        for (int i = 0; i < 19 ;i ++){
            if(!body_nodeActive[i]){
                body_nodes[i].transform.localPosition = new Vector3(0, -1, 0);
            }else{
                body_nodes[i].transform.localPosition = body_nodePos[i];
                if (ext){
                    if (i <= 1){
                        body_nodes[i].transform.localScale = new Vector3(0.02f, 0.02f, 0.02f);
                    }else if (i >= 13){
                        //body_nodes[i].transform.localScale = new Vector3(0.15f, 0.15f, 0.15f);
                        body_nodes[i].transform.localScale = new Vector3(bm.leg_width / 1000, bm.leg_width / 1000, bm.leg_width / 1000);
                    }else {
                        //body_nodes[i].transform.localScale = new Vector3(0.07f, 0.07f, 0.07f);
                        body_nodes[i].transform.localScale = new Vector3(bm.arm_width / 1000, bm.arm_width / 1000, bm.arm_width / 1000);
                    }
                }else {
                    body_nodes[i].transform.localScale = new Vector3(0.05f, 0.05f, 0.05f);
                }
            }
            //body_nodes[i].SetActive(body_nodeActive[i]);
            preNodes[i].transform.localPosition = prePose[i];
        }
        if (!body_nodeActive[8]){
            body_nodes[7].transform.localScale = new Vector3(0.02f, 0.02f, 0.02f); 
        }
        if (!body_nodeActive[12]){
            body_nodes[11].transform.localScale = new Vector3(0.02f, 0.02f, 0.02f); 
        }
        for (int i = 0; i < 21 ;i ++){
            if(!lh_nodeActive[i]){
                lh_nodes[i].transform.localPosition = new Vector3(0, -15, 0);
            }else{
                lh_nodes[i].transform.localPosition = lh_nodePos[i];
            }
            //lh_nodes[i].transform.localPosition = lh_nodePos[i];
            //lh_nodes[i].SetActive(lh_nodeActive[i]);
        }
        for (int i = 0; i < 21 ;i ++){
            if(!rh_nodeActive[i]){
                rh_nodes[i].transform.localPosition = new Vector3(0, -1, 0);
            }else{
                rh_nodes[i].transform.localPosition = rh_nodePos[i];
            }
            //rh_nodes[i].transform.localPosition = rh_nodePos[i];
            //rh_nodes[i].SetActive(rh_nodeActive[i]);
        }
        for (int i = 0; i < 18 ;i ++){
            Vector3 a = body_nodePos[bnMap[i].x], b = body_nodePos[bnMap[i].y];
            Vector3 forward = new Vector3(1, 1, 1);
            Vector3 upward = b - a;
            if (upward.x != 0){
                forward.x = -(upward.y + upward.z)/upward.x;
            }else if (upward.y != 0){
                forward.y = -(upward.x + upward.z)/upward.y;
            }else if (upward.z != 0){
                forward.z = -(upward.y + upward.x)/upward.z;
            }
            body_bones[i].SetActive(body_boneActive[i]);
            body_bones[i].transform.localPosition = (a + b) / 2;
            body_bones[i].transform.localRotation = Quaternion.LookRotation(forward, upward);
            if (ext){
                if (i >= 12){
                    //body_bones[i].transform.localScale = new Vector3(0.15f, 0.15f, 0.15f);
                    body_bones[i].transform.localScale = new Vector3(bm.leg_width / 1000, upward.magnitude / 2, bm.leg_width / 1000);
                }else{
                    //body_bones[i].transform.localScale = new Vector3(0.07f, 0.07f, 0.07f);
                    body_bones[i].transform.localScale = new Vector3(bm.arm_width / 1000, upward.magnitude / 2, bm.arm_width / 1000);
                }
            }else{
                body_bones[i].transform.localScale = new Vector3(0.02f, upward.magnitude / 2, 0.02f);
            }
        }

        //new model test
        if (ext){
            if (body_boneActive[0]){
                Vector3 a0 = body_nodePos[0], b0 = body_nodePos[1];
                head.SetActive(ext);
                head.transform.localPosition = (a0 + b0) / 2;
                //head.transform.localScale = new Vector3((a0 - b0).magnitude + 0.02f, (a0 - b0).magnitude + 0.04f, (a0 - b0).magnitude + 0.02f);
                head.transform.localScale = new Vector3(bm.head_width / 1000, bm.head_width / 1000 + 0.02f, bm.head_width / 1000);
            }
            if (body_boneActive[2]){
                Vector3 a2 = body_nodePos[4], b2 = body_nodePos[2];
                Vector3 _a2 = body_nodePos[9], _b2 = body_nodePos[5];
                shouder.transform.localPosition = (_a2 + _b2) / 2;
                body.transform.localPosition = (a2 + b2) / 2;
                shouder.SetActive(ext);
                body.SetActive(ext);
                shouder.transform.localRotation = Quaternion.LookRotation((_a2 - _b2), (a2 - b2));
                shouder.transform.localScale = new Vector3(0.22f, 0.06f, (_a2 - _b2).magnitude);
                body.transform.localRotation = Quaternion.LookRotation((_a2 - _b2), (a2 - b2));
                body.transform.localScale = new Vector3(0.2f, (a2 - b2).magnitude / 2, (_a2 - _b2).magnitude - 0.06f);
            }
            if (body_boneActive[3]){
                Vector3 a3 = body_nodePos[2], b3 = body_nodePos[3];
                Vector3 _a3 = body_nodePos[16], _b3 = body_nodePos[13];
                hip.transform.localPosition = (_a3 + _b3) / 2;
                waist.transform.localPosition = b3;
                waist.SetActive(ext);
                hip.SetActive(ext);
                hip.transform.localRotation = Quaternion.LookRotation((_a3 - _b3), (a3 - b3));
                //hip.transform.localScale = new Vector3(0.22f, 0.06f, (_a3 - _b3).magnitude + 0.16f);
                hip.transform.localScale = new Vector3(0.22f, 0.06f, bm.torso_width + 0.1f);
                waist.transform.localRotation = Quaternion.LookRotation((_a3 - _b3), (a3 - b3));
                //waist.transform.localScale = new Vector3(0.19f, (a3 - b3).magnitude / 1.1f, (_a3 - _b3).magnitude + 0.06f);
                waist.transform.localScale = new Vector3(0.19f, (a3 - b3).magnitude / 1.1f, bm.torso_width);
            }
        }else {
            head.SetActive(ext);
            shouder.SetActive(ext);
            body.SetActive(ext);
            waist.SetActive(ext);
            hip.SetActive(ext);
        }

        for (int i = 0; i < 21 ;i ++){
            Vector3 a = lh_nodePos[hbnMap[i].x], b = lh_nodePos[hbnMap[i].y];
            Vector3 forward = new Vector3(1, 1, 1);
            Vector3 upward = b - a;
            if (upward.x != 0){
                forward.x = -(upward.y + upward.z)/upward.x;
            }else if (upward.y != 0){
                forward.y = -(upward.x + upward.z)/upward.y;
            }else if (upward.z != 0){
                forward.z = -(upward.y + upward.x)/upward.z;
            }
            lh_bones[i].SetActive(lh_boneActive[i]);
            lh_bones[i].transform.localPosition = (a + b) / 2;
            lh_bones[i].transform.localRotation = Quaternion.LookRotation(forward, upward);
            lh_bones[i].transform.localScale = new Vector3(0.005f, upward.magnitude / 2, 0.005f); 
        }
        for (int i = 0; i < 21 ;i ++){
            Vector3 a = rh_nodePos[hbnMap[i].x], b = rh_nodePos[hbnMap[i].y];
            Vector3 forward = new Vector3(1, 1, 1);
            Vector3 upward = b - a;
            if (upward.x != 0){
                forward.x = -(upward.y + upward.z)/upward.x;
            }else if (upward.y != 0){
                forward.y = -(upward.x + upward.z)/upward.y;
            }else if (upward.z != 0){
                forward.z = -(upward.y + upward.x)/upward.z;
            }
            rh_bones[i].SetActive(rh_boneActive[i]);
            rh_bones[i].transform.localPosition = (a + b) / 2;
            rh_bones[i].transform.localRotation = Quaternion.LookRotation(forward, upward);
            rh_bones[i].transform.localScale = new Vector3(0.005f, upward.magnitude / 2, 0.005f); 
        }


        float cos = GetSimilarity_hand(okPose);
        if (currKeyPoseIndex >= keyPoseList.Length){
        }else if (cos > 0.91 && Time.time - last_ok_time > 5){
            last_ok_time = Time.time;
            NextKeyPos();
        }else{
            cos = GetSimilarity(keyPoseList[currKeyPoseIndex]);
            simiText.text = "当前步骤完成度：" + Mathf.Round(cos * 10000) / 100 + "%";
            if (keyPoseList[currKeyPoseIndex].type == "motion"){
                simText.text = "当前动作映射值：" + sim + "/" + keyPoseList[currKeyPoseIndex].motion_value;
                if (cos < 0.92){
                    int ret = Alert.updateAlertMsg(keyAlertId, "动作不规范：\n" + keyPoseList[currKeyPoseIndex].pose, 25);
                    if (ret != keyAlertId){
                        keyAlertId = ret;
                        LogLine("**！！！动作不规范：" + keyPoseList[currKeyPoseIndex].pose + "**");
                    }
                }else{
                    Alert.removeAlertMsg(keyAlertId);
                }
                
                if (currKeyPoseIndex + 1 < keyPoseList.Length){
                    int i = currKeyPoseIndex + 1;
                    cos = GetSimilarity(keyPoseList[i]);
                    if (cos > 0.98 && keyPoseList[i].type == "check"){
                        Alert.removeAlertMsg(keyAlertId);
                        NextKeyPos();
                    }
                }
            }else if (keyPoseList[currKeyPoseIndex].type == "ppe"){
                if (true){
                    if (PPED.has_glove){
                        glove_left.transform.localPosition = body_nodePos[8];
                        glove_right.transform.localPosition = body_nodePos[12];
                        int ret = Alert.updateAlertMsg(ppe_gloveAlertId, "请立即摘下手套！", 110);
                        if (ret != ppe_gloveAlertId){
                            ppe_gloveAlertId = ret;
                            LogLine("**！！！佩戴手套**");
                        }
                    }else{
                        glove_left.transform.localPosition = new Vector3(1.65f, 1.24f, 0.7f);
                        glove_right.transform.localPosition = new Vector3(1.45f, 1.24f, 0.7f);
                        Alert.removeAlertMsg(ppe_gloveAlertId);
                        ppe_gloveAlertId = -1;
                    }
                }
                if (keyPoseList[currKeyPoseIndex].pose.Contains("护目镜")){
                    if (PPED.has_goggle){
                        goggle.transform.localPosition = body_nodePos[0];
                        Alert.removeAlertMsg(ppe_goggleAlertId);
                        ppe_goggleAlertId = -1;
                        NextKeyPos();
                    }else if(body_nodeActive[0]){
                        goggle.transform.localPosition = new Vector3(1.6f, 1.25f, 0.5f);
                        int ret = Alert.updateAlertMsg(ppe_goggleAlertId, "是否佩戴护目镜？", 101);
                        if (ret != ppe_goggleAlertId){
                            ppe_goggleAlertId = ret;
                            LogLine("**！！！未佩戴护目镜**");
                        }
                    }
                }else if (keyPoseList[currKeyPoseIndex].pose.Contains("工作帽")){
                    if (PPED.has_helmet){
                        helmet.transform.localPosition = body_nodePos[0] + new Vector3(0, 0.11f, 0);
                        Alert.removeAlertMsg(ppe_helmetAlertId);
                        ppe_helmetAlertId = -1;
                        NextKeyPos();
                    }else if(body_nodeActive[0]){
                        helmet.transform.localPosition = new Vector3(1.25f, 1.28f, 0.5f);
                        int ret = Alert.updateAlertMsg(ppe_helmetAlertId, "是否佩戴工作帽？", 101);
                        if (ret != ppe_helmetAlertId){
                            ppe_helmetAlertId = ret;
                            LogLine("**！！！未佩戴工作帽**");
                        }
                    }
                }
            }else {                
                simText.text = "当前动作映射值：" + sim;
                if (cos > 0.97){
                    Alert.removeAlertMsg(keyAlertId);
                    NextKeyPos();
                }
                if (currKeyPoseIndex + 1 < keyPoseList.Length){
                    int i = currKeyPoseIndex + 1;
                    cos = GetSimilarity(keyPoseList[i]);
                    if (cos > 0.98 && keyPoseList[i].type != "motion"){
                        int ret = Alert.updateAlertMsg(keyAlertId, "遗漏重要步骤：\n" + keyPoseList[currKeyPoseIndex].pose, 35);
                        if (ret != keyAlertId){
                            keyAlertId = ret;
                            LogLine("**！！！遗漏了重要步骤：" + keyPoseList[currKeyPoseIndex].pose + "**");
                        }
                    }
                }
            }            
        }

        if (ppe_goggleAlertId == -1){
            goggle.transform.localPosition = body_nodePos[0];
        }
        if (ppe_helmetAlertId == -1){
            helmet.transform.localPosition = body_nodePos[0] + new Vector3(0, 0.11f, 0);
        }
        if (ppe_gloveAlertId == -1){
            glove_left.transform.localPosition = new Vector3(1.65f, 1.24f, 0.7f);
            glove_right.transform.localPosition = new Vector3(1.45f, 1.24f, 0.7f);
        }


        if (LatheStatus.running != machine_running){
            LatheStatus.running = machine_running;
            foreach (GameObject obj in dangerAreas)
            {
                if (obj.name.Contains("static")) continue;
                Vector3 old_pos = obj.transform.localPosition;
                if (machine_running){
                    //collider.enabled = true;
                    obj.GetComponent<MeshRenderer>().material = dangerMat;
                    obj.transform.localPosition = old_pos + new Vector3(0, -15, 0);
                }else{
                    //collider.enabled = false;
                    obj.gameObject.GetComponent<MeshRenderer>().material = preMat;
                    obj.transform.localPosition = old_pos + new Vector3(0, 15, 0);
                }
            }
            // foreach (Transform child in currentObject.transform)
            // {
            //     if(child.gameObject.name.Contains("dangerCylinder")){
            //         //CapsuleCollider collider = child.gameObject.GetComponent<CapsuleCollider>();
            //         Vector3 old_pos = child.localPosition; //
            //         if (machine_running){
            //             //collider.enabled = true;
            //             child.gameObject.GetComponent<MeshRenderer>().material = dangerMat;
            //             child.localPosition = old_pos + new Vector3(0, -15, 0);
            //         }else{
            //             //collider.enabled = false;
            //             child.gameObject.GetComponent<MeshRenderer>().material = preMat;
            //             child.localPosition = old_pos + new Vector3(0, 15, 0);
            //         }
            //     }
            //     if(child.gameObject.name.Contains("dangerCube")){
            //         //BoxCollider collider = child.gameObject.GetComponent<BoxCollider>();
            //         Vector3 old_pos = child.localPosition; //
            //         if (machine_running){
            //             //collider.enabled = true;
            //             child.gameObject.GetComponent<MeshRenderer>().material = dangerMat;
            //             child.localPosition = old_pos + new Vector3(0, -15, 0);
            //         }else{
            //             //collider.enabled = false;
            //             child.gameObject.GetComponent<MeshRenderer>().material = preMat;
            //             child.localPosition = old_pos + new Vector3(0, 15, 0);
            //         }
            //     }
            // }
        }

        timeText.text = string.Format("{0:T}", DateTime.Now);
    }

    private void OnApplicationQuit() {
        LogLine("**iGuard Quit**");
        log_file.Close();
    }

    static public void LogLine(string l){
        log_file.WriteLine("[" + string.Format("{0:T}", DateTime.Now)+ "] " + l);
    }

    private float GetSimilarity(KeyPack_NodeInfo kn){
        //Debug.Log("sim: " + kn.type + kn.pose);
        float s;
        if (kn.type == "key"){
            for (int i = 0; i < 19 ;i ++){
                keyPose[i].x = kn.nodes[i].x / 1000;
                keyPose[i].y = kn.nodes[i].y / 1000;
                keyPose[i].z = kn.nodes[i].z / 1000;
            }
            s = GetSimilarity_body(keyPose);
        }else if (kn.type == "motion"){
            s = GetSimilarity_motion(kn.motion_value);
        }else{
            if (LatheStatus.running == kn.running && LatheStatus.carriage_x <= kn.carriage_x_high && LatheStatus.carriage_x >= kn.carriage_x_low){
                return 1f;
            }
            return 0f;
        }
        //Debug.Log("sim: " + s);
        return s;
    }

    private float GetSimilarity_body(Vector3[] kp){
        float r = 0;
        float m1 = 0, m2 = 0;
        for (int i = 0; i < 12; i ++){
            if (i == 7 || i == 11) continue;
            Vector3 a = body_nodePos[bnMap[i].x], b = body_nodePos[bnMap[i].y];
            Vector3 ak = kp[bnMap[i].x], bk = kp[bnMap[i].y];
            Vector3 upward = (b - a), upwardk = (bk - ak).normalized * (b - a).magnitude;
            r += upward.x * upwardk.x;
            r += upward.y * upwardk.y;
            r += upward.z * upwardk.z;
            m1 += upward.magnitude * upward.magnitude;
            m2 += upwardk.magnitude * upwardk.magnitude;
        }
        float cos = r / Mathf.Sqrt(m1) / Mathf.Sqrt(m2);
        return cos * cos;
    }
    private float GetSimilarity_motion(float value){
        float cos = 2 * value * sim / ((value * value) + (sim * sim));
        return cos * cos * cos * cos;
    }

    private float GetSimilarity_hand(Vector3[] kp){
        float r = 0;
        float m1 = 0, m2 = 0;
        for (int i = 0; i < 21; i ++){
            Vector3 a = rh_nodePos[hbnMap[i].x], b = rh_nodePos[hbnMap[i].y];
            Vector3 ak = kp[hbnMap[i].x], bk = kp[hbnMap[i].y];
            Vector3 upward = (b - a), upwardk = (bk - ak).normalized * (b - a).magnitude;
            r += upward.x * upwardk.x;
            r += upward.y * upwardk.y;
            r += upward.z * upwardk.z;
            m1 += upward.magnitude * upward.magnitude;
            m2 += upwardk.magnitude * upwardk.magnitude;
        }
        float cos = r / Mathf.Sqrt(m1) / Mathf.Sqrt(m2);
        return cos * cos;
    }



    private void NextKeyPos(){
        if (currKeyPoseIndex >= 0){
            OKMsg.addAlertMsg("已完成：" + keyPoseList[currKeyPoseIndex].pose);
            DangerCol.SendMsg("G");
            LogLine("**Pass CheckPoint: " + keyPoseList[currKeyPoseIndex].pose + "**");
        }
        currKeyPoseIndex ++;
        if (currKeyPoseIndex < keyPoseList.Length){
            KeyPack_NodeInfo node_info = keyPoseList[currKeyPoseIndex];
            // foreach (Transform child in currentObject.transform)
            // {
            //     if(child.gameObject.name.Contains("dangerCylinder")){
            //         CapsuleCollider collider = child.gameObject.GetComponent<CapsuleCollider>();
            //         if (node_info.danger){
            //             collider.enabled = true;
            //             child.gameObject.GetComponent<MeshRenderer>().material = dangerMat;
            //         }else{
            //             collider.enabled = false;
            //             child.gameObject.GetComponent<MeshRenderer>().material = preMat;
            //         }
            //     }
            //     if(child.gameObject.name.Contains("dangerCube")){
            //         BoxCollider collider = child.gameObject.GetComponent<BoxCollider>();
            //         if (node_info.danger){
            //             collider.enabled = true;
            //             child.gameObject.GetComponent<MeshRenderer>().material = dangerMat;
            //         }else{
            //             collider.enabled = false;
            //             child.gameObject.GetComponent<MeshRenderer>().material = preMat;
            //         }
            //     }
            // }
            // if (!node_info.danger){
            //     DangerCol.noDanger();
            // }
            nextText.text = "请执行关键步骤("+ (currKeyPoseIndex + 1) +"/"+ keyPoseList.Length +")：" + node_info.pose;
            actText.text = "当前应做动作：" + node_info.motion_name;
        }else {
            nextText.text = "无关键步骤信息";
            actText.text = "无应做动作";
            // foreach (Transform child in currentObject.transform)
            // {
            //     if(child.gameObject.name.Contains("dangerCylinder")){
            //         CapsuleCollider collider = child.gameObject.GetComponent<CapsuleCollider>();
            //         collider.enabled = false;
            //         child.gameObject.GetComponent<MeshRenderer>().material = preMat;
            //     }
            //     if(child.gameObject.name.Contains("dangerCube")){
            //         BoxCollider collider = child.gameObject.GetComponent<BoxCollider>();
            //         collider.enabled = false;
            //         child.gameObject.GetComponent<MeshRenderer>().material = preMat;
            //     }
            // }
        }
    }

    private void LoadKeyPos(){
        foreach(Transform child in edit_panel.transform){
            if (child.gameObject.name.Contains("btn")){
                Destroy(child.gameObject);
            }
        }
        for (int i = 0; i < keyPoseNameList.Length; i++){
            Debug.Log("loading key pose " + keyPoseNameList[i]);
            StreamReader sr = new StreamReader("./Assets/Pose/" + keyPoseNameList[i] + ".json", Encoding.UTF8);
            string content =  sr.ReadToEnd();
            sr.Close();
            KeyPack_NodeInfo node_info = JsonUtility.FromJson<KeyPack_NodeInfo>(content);
            keyPoseList[i] = node_info;
            Button btn = GameObject.Instantiate(btnPrefab);
            btn.transform.parent = edit_panel.transform;
            btn.transform.localPosition = new Vector3(0, 410 - i * 70, 0);
            btn.name = "btn" + i;
            int tmp = i;
            btn.onClick.AddListener(()=>{
                this.SetSelect(tmp);
            });
            btn.transform.Find("Text (Legacy)").GetComponent<Text>().text = keyPoseNameList[i];
        }
    }

    private void LoadOKPos(){
        StreamReader sr = new StreamReader("./Assets/Pose/ok/ok.json", Encoding.UTF8);
        string content =  sr.ReadToEnd();
        sr.Close();
        KeyPack_NodeInfo node_info = JsonUtility.FromJson<KeyPack_NodeInfo>(content);
        for (int i = 0; i < 21 ;i ++){
            okPose[i].x = node_info.nodes[i].x / 1000;
            okPose[i].y = node_info.nodes[i].y / 1000;
            okPose[i].z = node_info.nodes[i].z / 1000;
        }
    }

    public void SetoKey(){
        for (int idx = 0; idx < keyPoseList.Length; idx ++){
            KeyPack_NodeInfo keypos = keyPoseList[(idx + currKeyPoseIndex) % keyPoseList.Length];
            if (keypos.type == "key"){
                Debug.Log("set to " + keypos.pose);
                WebPack_NodeInfo ni = new WebPack_NodeInfo();
                last_frame_id = 0;
                ni.frame_id = 1;
                ni.body_nodes = new WebPack_Vector3[19];
                for (int i = 0; i < 19 ;i ++){
                    ni.body_nodes[i] = new WebPack_Vector3();
                    ni.body_nodes[i].x = keypos.nodes[i].x;
                    ni.body_nodes[i].y = keypos.nodes[i].y;
                    ni.body_nodes[i].z = keypos.nodes[i].z;
                    ni.body_nodes[i].score = 100;
                }
                SetNodePos(ni);
                currKeyPoseIndex = (idx + currKeyPoseIndex) % keyPoseList.Length - 1;
                NextKeyPos();
                break;
            }
        }
    }

    private void SetNodePos(WebPack_NodeInfo ni){

        if (last_frame_id > ni.frame_id) return;

        if (last_frame_id == ni.frame_id){

        }else{
            for (int i = 0; i < 19 ;i ++){
                body_lastNodePos[i] = body_nodePos[i];
                body_nodeActive[i] = (ni.body_nodes[i].score != 0);
                //body_nodeActive[i] = true;
                body_nodePos[i].x = body_nodePos[i].x * (1 - ni.body_nodes[i].score / 100) + (ni.body_nodes[i].x / 1000 * ni.body_nodes[i].score / 100);
                body_nodePos[i].y = body_nodePos[i].y * (1 - ni.body_nodes[i].score / 100) + (ni.body_nodes[i].y / 1000 * ni.body_nodes[i].score / 100);
                body_nodePos[i].z = body_nodePos[i].z * (1 - ni.body_nodes[i].score / 100) + (ni.body_nodes[i].z / 1000 * ni.body_nodes[i].score / 100);
                // body_nodePos[i].x = ni.body_nodes[i].x / 1000;
                // body_nodePos[i].y = ni.body_nodes[i].y / 1000;
                // body_nodePos[i].z = ni.body_nodes[i].z / 1000;
            }
            for (int i = 0; i < 18 ;i ++){
                body_boneActive[i] = (ni.body_nodes[bnMap[i].x].score != 0 && ni.body_nodes[bnMap[i].y].score != 0);
                //body_boneActive[i] = true;
            }
            if (ni.left_hand_nodes != null){
                body_nodeActive[8] = false;
                body_boneActive[7] = false;
                for (int i = 0; i < 21;i ++){
                    lh_lastNodePos[i] = lh_nodePos[i];
                    lh_nodeActive[i] = (ni.left_hand_nodes[i].score != 0);
                    lh_nodePos[i].x = lh_nodePos[i].x * (1 - ni.left_hand_nodes[i].score / 100) + (ni.left_hand_nodes[i].x / 1000 * ni.left_hand_nodes[i].score / 100);
                    lh_nodePos[i].y = lh_nodePos[i].y * (1 - ni.left_hand_nodes[i].score / 100) + (ni.left_hand_nodes[i].y / 1000 * ni.left_hand_nodes[i].score / 100);
                    lh_nodePos[i].z = lh_nodePos[i].z * (1 - ni.left_hand_nodes[i].score / 100) + (ni.left_hand_nodes[i].z / 1000 * ni.left_hand_nodes[i].score / 100);
                }
                for (int i = 0; i < 21 ;i ++){
                    lh_boneActive[i] = (ni.left_hand_nodes[hbnMap[i].x].score != 0 && ni.left_hand_nodes[hbnMap[i].y].score != 0);
                }
            }else {
                for (int i = 0; i < 21;i ++){
                    lh_nodeActive[i] = false;
                }
                for (int i = 0; i < 21 ;i ++){
                    lh_boneActive[i] = false;
                }
            }
            if (ni.right_hand_nodes != null){
                body_nodeActive[12] = false;
                body_boneActive[11] = false;
                for (int i = 0; i < 21 ;i ++){
                    rh_lastNodePos[i] = rh_nodePos[i];
                    rh_nodeActive[i] = (ni.right_hand_nodes[i].score != 0);
                    rh_nodePos[i].x = rh_nodePos[i].x * (1 - ni.right_hand_nodes[i].score / 100) + (ni.right_hand_nodes[i].x / 1000 * ni.right_hand_nodes[i].score / 100);
                    rh_nodePos[i].y = rh_nodePos[i].y * (1 - ni.right_hand_nodes[i].score / 100) + (ni.right_hand_nodes[i].y / 1000 * ni.right_hand_nodes[i].score / 100);
                    rh_nodePos[i].z = rh_nodePos[i].z * (1 - ni.right_hand_nodes[i].score / 100) + (ni.right_hand_nodes[i].z / 1000 * ni.right_hand_nodes[i].score / 100);
                }
                for (int i = 0; i < 21 ;i ++){
                    rh_boneActive[i] = (ni.right_hand_nodes[hbnMap[i].x].score != 0 && ni.right_hand_nodes[hbnMap[i].y].score != 0);
                }
            }else {
                for (int i = 0; i < 21;i ++){
                    rh_nodeActive[i] = false;
                }
                for (int i = 0; i < 21 ;i ++){
                    rh_boneActive[i] = false;
                }
            }

            LatheStatus.carriage_x = ni.carriage_x / 100;
            LatheStatus.carriage_z = ni.carriage_z / 100;
            machine_running = ni.running;

            bm = ni.body_metrics;

            // if (last_frame_id != -1){
            //     for (int i = 0; i < 19 ;i ++){
            //         prePose[i].x = (body_nodePos[i].x - body_lastNodePos[i].x) < 0.1 ? (2 * body_nodePos[i].x - body_lastNodePos[i].x) : body_lastNodePos[i].x;
            //         prePose[i].y = (body_nodePos[i].y - body_lastNodePos[i].y) < 0.1 ? (2 * body_nodePos[i].y - body_lastNodePos[i].y) : body_lastNodePos[i].y;
            //         prePose[i].z = (body_nodePos[i].z - body_lastNodePos[i].z) < 0.1 ? (2 * body_nodePos[i].z - body_lastNodePos[i].z) : body_lastNodePos[i].z;
            //     }
            // }
            last_frame_id = ni.frame_id;
        }
    }

    void receiveFromClient(){
        int recv;
        byte[] data;
        IPEndPoint sender = new IPEndPoint(IPAddress.Any, 0);
        EndPoint Remote = (EndPoint)(sender);
        while(true){
            data = new byte[10240];
            recv = socket.ReceiveFrom(data, ref Remote);
            //Debug.Log(recv.ToString() + " bytes received from " + Remote.ToString() + ":");
            string node_info_str = Encoding.UTF8.GetString(data, 0, recv);
            Debug.Log(node_info_str);

            WebPack_NodeInfo node_info = JsonUtility.FromJson<WebPack_NodeInfo>(node_info_str);
            //Debug.Log("recv: " + node_info.carriage_x);
            //Debug.Log(node_info.nodes);
            SetNodePos(node_info);
        }
    }
    public void CaseChanged(int value){
        Debug.Log("Change to mode " + value);
        lathe.SetActive(value == 0);
        driller.SetActive(value == 1);
        if (value == 0){
            currentObject = lathe;
            keyPoseNameList = new string[9]{"lathe/ppe_goggle", "lathe/ppe_helmet", "lathe/tighten_knife", "lathe/tighten", "lathe/start", "lathe/check", "lathe/front", "lathe/stop", "lathe/back"};
            keyPoseList = new KeyPack_NodeInfo[9];
            LoadKeyPos();
            currKeyPoseIndex = -1;
            NextKeyPos();
        }else if (value == 1){
            currentObject = driller;
            keyPoseNameList = new string[5]{"driller/wrench", "driller/start", "driller/drill", "driller/stop", "driller/release"};
            keyPoseList = new KeyPack_NodeInfo[5];
            LoadKeyPos();
            currKeyPoseIndex = -1;
            NextKeyPos();
        }
    }

    public void DangerModeChanged(bool f){
        dangerText.enabled = f;
        for (int i = 0; i < 19 ;i ++){
            body_nodes[i].GetComponent<SphereCollider>().enabled = f;
        }
        for (int i = 0; i < 21 ;i ++){
            lh_nodes[i].GetComponent<SphereCollider>().enabled = f;
        }
        for (int i = 0; i < 19 ;i ++){
            rh_nodes[i].GetComponent<SphereCollider>().enabled = f;
        }
        if (!f){
            DangerCol.noDanger();
        }
    }

    public void KeyModeChanged(bool f){
        simiText.enabled = f;
        nextText.enabled = f;
        if (f){
            currKeyPoseIndex = -1;
            NextKeyPos();
        }
    }

    public void ActionModeChanged(bool f){
        actText.enabled = f;
        simText.enabled = f;
    }

    public void CameraChanged(int m){
        if (m == 0){
            camera_relative_pos = new Vector3(1.8f, -1.2f, 0.8f);
        }else if (m == 3){
            camera_relative_pos = new Vector3(0f, -1.3f, 1.4f);
        }else if (m == 2){
            camera_relative_pos = new Vector3(0f, -0.6f, -1.8f);
        }else {
            camera_relative_pos = new Vector3(0f, -2.6f, -0.2f);
        }
        Camera.main.transform.localPosition = body_nodePos[3] - camera_relative_pos;
        Camera.main.transform.localRotation = Quaternion.LookRotation(camera_relative_pos, new Vector3(0, 1, 0));
    }
    public void ChangeEXT(){
        ext = !ext;
    }
    public void ChangeEDIT(){
        edit_panel.SetActive(!edit_panel.activeSelf);
    }
    public void ResetAll(){
        DangerModeChanged(false);
        ActionModeChanged(false);
        KeyModeChanged(false);
        keyAlertId = -1;
        ppe_helmetAlertId = ppe_gloveAlertId = ppe_goggleAlertId = -2;
        machine_running = false;
        DangerCol.noDanger();
        Alert.removeAll();
        DangerModeChanged(true);
        ActionModeChanged(true);
        KeyModeChanged(true);
    }

    public void AddKey(){
        Debug.Log("add: " + selectIndex);
        int len = keyPoseNameList.Length;
        string[] new_name_list = new string[len + 1];
        int j = 0;
        for(int i = 0; i < len + 1; i++){
            if (i == selectIndex){
                new_name_list[i] = input_text.text;
            }else{
                new_name_list[i] = keyPoseNameList[j];
                j++;
            }
        }
        keyPoseNameList = new_name_list;
        keyPoseList = new KeyPack_NodeInfo[len + 1];
        LoadKeyPos();
        currKeyPoseIndex = -1;
        NextKeyPos();
        LogLine("**插入了操作步骤: " + keyPoseList[selectIndex].pose + "**");
    }
    public void DelKey(){
        Debug.Log("delete: " + selectIndex);
        LogLine("**删除了操作步骤: " + keyPoseList[selectIndex].pose + "**");
        int len = keyPoseNameList.Length;
        string[] new_name_list = new string[len - 1];
        int j = 0;
        for(int i = 0; i < len; i++){
            if (i == selectIndex){
            }else{
                new_name_list[j] = keyPoseNameList[i];
                j++;
            }
        }
        keyPoseNameList = new_name_list;
        keyPoseList = new KeyPack_NodeInfo[len - 1];
        LoadKeyPos();
        currKeyPoseIndex = -1;
        NextKeyPos();
    }

    public void SetSelect(int sel){
        Debug.Log("set: " + sel);
        selectIndex = sel;
    }
}
