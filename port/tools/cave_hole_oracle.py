#!/usr/bin/env python3
"""Conservative source-alpha hole screen on retained emitted Forest GL quads.

This is an offline candidate oracle, not an implementation-dependent NVE7
rasterizer: it does not certify triangle coverage, interpolated UV or pixels.
"""
import argparse
import json
import struct
from pathlib import Path
from PIL import Image, ImageFilter

IDS = (53, 54, 55, 60, 61)
VIEW = (1280, 720)

def largest(mask, w, h):
    heights = [0] * w
    best = (0, (0, 0, 0, 0))
    for y in range(h):
        row = mask[y * w:(y + 1) * w]
        for x, v in enumerate(row):
            heights[x] = heights[x] + 1 if v else 0
        stack = []
        for x in range(w + 1):
            height = heights[x] if x < w else 0
            start = x
            while stack and stack[-1][1] > height:
                left, old = stack.pop()
                area = (x - left) * old
                if area > best[0]:
                    best = (area, (left, y + 1 - old, x, y + 1))
                start = left
            if not stack or stack[-1][1] < height:
                stack.append((start, height))
    return best

def point(q, u, v):
    a, b, c = q
    return (a[0] + u*(b[0]-a[0]) + v*(c[0]-a[0]),
            a[1] + u*(b[1]-a[1]) + v*(c[1]-a[1]))

def clip(poly, axis, edge, above):
    def val(p): return (p[axis]-edge) if above else (edge-p[axis])
    out = []
    for p, q in zip(poly, poly[1:] + poly[:1]):
        a, b = val(p), val(q)
        if a >= 0: out.append(p)
        if (a < 0 < b) or (b < 0 < a):
            t = a/(a-b)
            out.append((p[0]+t*(q[0]-p[0]), p[1]+t*(q[1]-p[1])))
    return out

def area(poly):
    return abs(sum(a[0]*b[1]-b[0]*a[1] for a,b in zip(poly, poly[1:]+poly[:1])))/2 if len(poly)>2 else 0

def visible_area(q, r, w, h):
    x0,y0,x1,y1 = r
    poly = [point(q,x0/w,y0/h),point(q,x1/w,y0/h),point(q,x1/w,y1/h),point(q,x0/w,y1/h)]
    for axis, edge, above in ((0,0,True),(0,VIEW[0],False),(1,0,True),(1,VIEW[1],False)):
        if not poly: break
        poly = clip(poly,axis,edge,above)
    return area(poly)

def main():
    p=argparse.ArgumentParser()
    p.add_argument('--image', required=True)
    p.add_argument('--xy-blob',required=True)
    p.add_argument('--uv-blob',required=True)
    p.add_argument('--rectangles',type=int,default=8)
    a=p.parse_args()
    im=Image.open(a.image).convert('RGBA').getchannel('A')
    w,h=im.size
    # A 3x3 max filter requires the source texel and every immediate neighbor
    # to be alpha zero. Edges are rejected separately: GL clamp/filter behavior
    # must not be inferred from PIL's border extension.
    safe=im.filter(ImageFilter.MaxFilter(3)).point(lambda v: 255 if v == 0 else 0)
    mask=bytearray(safe.tobytes())
    mask[:w]=bytes(w);mask[(h-1)*w:]=bytes(w)
    for y in range(h):mask[y*w]=0;mask[y*w+w-1]=0
    b=Path(a.xy_blob).read_bytes(); uv=Path(a.uv_blob).read_bytes()
    quads=[]
    for i in range(5):
        xy=[struct.unpack_from('<2f',b,(i*6+j)*20) for j in (0,1,2)]
        tc=[struct.unpack_from('<2f',uv,(i*6+j)*20) for j in (0,1,2)]
        if tc != [(0.,0.),(1.,0.),(0.,1.)]:raise SystemExit(f'unexpected UV {i}: {tc}')
        quads.append(xy)
    rects=[]
    for i in range(a.rectangles):
        _,r=largest(mask,w,h)
        if r[0]==r[2]:break
        x0,y0,x1,y1=r
        counts=[]
        for qq in quads:
            counts.append(round(visible_area(qq,r,w,h),2))
        rects.append({'source_rect':r,'source_area':(x1-x0)*(y1-y0),
                      'visible_area_px2_per_quad':counts,
                      'visible_area_px2_total':round(sum(counts),2)})
        for y in range(y0,y1):mask[y*w+x0:y*w+x1]=bytes(x1-x0)
    total=sum(visible_area(q,(0,0,w,h),w,h) for q in quads)
    print(json.dumps({'source':a.image,'size':[w,h], 'emitted_gl_ids':IDS,
       'guarded_safe_texels':safe.histogram()[255],
       'visible_five_quad_area_px2':round(total,2),
       'rectangles':rects,
       'prefix': [{'n':n,'visible_area_px2':round(sum(r['visible_area_px2_total'] for r in rects[:n]),2),
                   'fraction_of_visible_area':round(sum(r['visible_area_px2_total'] for r in rects[:n])/total,5),
                   'naive_additional_vertices_all_five':n*5*18}
                  for n in (1,2,4,8) if n<=len(rects)]},indent=2))
if __name__=='__main__': main()
