using System.Collections;
using System.Collections.Generic;
using System.Linq;
using UnityEngine;
using DG.Tweening;


public class Macb : MonoBehaviour
{


    public Transform origin;
    public GameObject cubePrefab;
    public Texture2D[] templates;
    public List<List<Vector3>> targets;


    private List<GameObject> cubes = new List<GameObject>();


    void Start()
    {
        var initPos = new List<Vector3>();
        for (var i = 0; i < 256; i++)
        {
            var cube = Object.Instantiate(cubePrefab);
            cube.transform.SetParent(origin);
            int x = i % 16;
            int y = i / 16;
            var p = new Vector3((x - 7.5f) * 0.07f, 0, (y - 7.5f) * 0.07f);
            cube.transform.localPosition = p;
            initPos.Add(p);
            cubes.Add(cube);
        }

        targets = templates.Select(t => BuildTarget(t)).ToList();
        targets.Insert(0, initPos);
    }


    List<Vector3> BuildTarget(Texture2D tex)
    {
        Debug.Log($"{tex.width}, {tex.height}");
        var pixels = tex.GetPixels32();
        Debug.Log(pixels.Length);
        var pos = new List<Vector3>();
        var i = 0;
        foreach (var c in pixels)
        {
            if (c.r > 127)
            {
                int x = i % tex.width;
                int y = i / tex.height;
                pos.Add(new Vector3((x - (tex.width / 2f)) * 0.04f, 0, (y - (tex.height / 2f)) * 0.04f));
            }
            i++;
        }
        Debug.Log(pos.Count);
        return pos;
    }


    int current = 0;

    void Update()
    {
        if (Input.GetKeyDown(KeyCode.Space))
        {
            var target = targets[++current % targets.Count];
            target = target.OrderBy(a => System.Guid.NewGuid()).ToList();
            for (var i = 0; i < cubes.Count; i++)
            {
                var c = cubes[i];
                var t = target[i];
                var distance = Vector3.Distance(c.transform.localPosition, t);
                var angle = Vector3.Angle(c.transform.TransformDirection(Vector3.forward), t - c.transform.localPosition);
                var d = angle / 90f;
                c.transform.DOLookAt(origin.TransformPoint(t), d).SetDelay(Random.value);
                c.transform.DOLocalMove(t, distance / 0.1f)
                    .SetEase(Ease.Linear)
                    .SetDelay(d);
            }
        }
    }

}
