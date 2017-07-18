
UWP XAML 3D Model Control

DXModel is loading .cmo models formate to convert your model to .cmo just create new c++ directx project then add your 3d model to it and build the project you will find the cmo file, the compiled shader and .dds texture in the project pin folder copy them and put them in your project root folder and pass the model name to the dxmodel xaml control check the test sample for more details

 <dx:DXModel Height="300"
             ModelName="Teapot.cmo"
             Width="300"
             ModelLoaded="OnModelLoaded"/>

features

1-Transfom (translation,rotation,Scale)

2-Camera Control

comming.. 

2-changing the texture

1-changing the material 

3-controlling the light 
