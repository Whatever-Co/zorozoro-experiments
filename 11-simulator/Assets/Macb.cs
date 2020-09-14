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


    class MoveInfo
    {
        public Transform cube;
        public Vector3 target;
        public float distance;
    }


    void Move(List<Vector3> target)
    {
        var info = cubes.Zip(target.OrderBy(a => System.Guid.NewGuid()), (c, t) =>
        {
            return new MoveInfo
            {
                cube = c.transform,
                target = t,
                distance = Vector3.Distance(c.transform.localPosition, t)
            };
        }).OrderBy(i => i.distance).ToList();
        var totalDistance = info.Aggregate(0f, (t, i) => t += i.distance);
        Debug.Log(totalDistance);
        for (var j = 0; j < 100; j++)
        // while (true)
        {
            for (var i = 0; i < cubes.Count / 2; i++)
            {
                var c0 = info[i];
                var c1 = info[cubes.Count - i - 1];
                var d0 = c0.distance + c1.distance;
                var e0 = Vector3.Distance(c0.cube.transform.localPosition, c1.target);
                var e1 = Vector3.Distance(c1.cube.transform.localPosition, c0.target);
                var e2 = e0 + e1;
                // Debug.Log($"d0={d0}, e0={e0}, e1={e1}, e2={e2}");
                if (e2 < d0)
                {
                    var tmp = c0.target;
                    c0.target = c1.target;
                    c1.target = tmp;
                    c0.distance = Vector3.Distance(c0.cube.transform.localPosition, c0.target);
                    c1.distance = Vector3.Distance(c1.cube.transform.localPosition, c1.target);
                }
            }
            var dist = info.Aggregate(0f, (t, i) => t += i.distance);
            Debug.Log($"{j}, {dist}");
            if (dist >= totalDistance)
            {
                break;
            }
            totalDistance = dist;
            info = info.OrderBy(i => i.distance).ToList();
        }

        foreach (var i in info)
        {
            var c = i.cube;
            var t = i.target;
            var angle = Vector3.Angle(c.transform.TransformDirection(Vector3.forward), t - c.transform.localPosition);
            var d = angle / 90f;
            c.transform.DOLookAt(origin.TransformPoint(t), d).SetDelay(Random.value);
            c.transform.DOLocalMove(t, i.distance / 0.1f)
                .SetEase(Ease.Linear)
                .SetDelay(d);
        }
    }


    int current = 0;

    void Update()
    {
        if (Input.GetKeyDown(KeyCode.Space))
        {
            Move(targets[++current % targets.Count]);
        }
    }

}
