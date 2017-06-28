# UnityLz4
Decompressor for LZ4 Format AssetBundle Files (.unity3d.lz4)

- How to compile
```
g++ lz4.cpp -o lz4
```

- How to use
```
./lz4 [Path to file]
```
[File Name].dec will be exported.

## What is LZ4 Format?
Unity supports LZ4 compression, which results in larger compressed file sizes, but does not require the entire bundle to be decompressed before use. LZ4 is a "chunk-based" algorithm, and therefore when objects are loaded from an LZ4-compressed bundle, only the corresponding chunks for that object are decompressed. This occurs on-the-fly, meaning there are no wait times for the entire bundle to be decompressed before use. The LZ4 Format was introduced in Unity 5.3 and was unavailable in prior versions.  
(From [Unity Docs](https://docs.unity3d.com/530/Documentation/Manual/AssetBundleCompression.html))

## Thanks to Grain
I created this source code from JavaScript one, which Grain made:  
http://grain.exout.net/lz4/run.html
