using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class DangerCol : MonoBehaviour
{

    public Text dangerText;

    // Start is called before the first frame update
    void Start()
    {
        dangerText.text = "当前无危险";
    }

    // Update is called once per frame
    void Update()
    {
        
    }

    private void OnCollisionEnter(Collision other) {
        Debug.Log("Danger!");
        if (other.gameObject.name.Contains("node")){
            dangerText.text = "！！误触危险区：" + other.gameObject.name;
        }else if (other.gameObject.name.Contains("pNode")){
            dangerText.text = "可能进入危险区：" + other.gameObject.name;
        }
    }

    private void OnCollisionExit(Collision other) {
        if (dangerText.text.Contains(other.gameObject.name)){
            dangerText.text = "当前无危险";
        }
    }
}
