A uwp xaml control to render 3d model.

DXModel is loading .cmo models format. To convert your model to .cmo just create new c++ directx project then add your 3d model to it and build the project you will find the cmo file, the compiled shader and .dds texture in the project pin folder copy them to your project root folder and pass the model name to the dxmodel xaml control check the test sample for more details

 <dx:DXModel Height="300"
             ModelName="Teapot.cmo"
             Width="300"
             ModelLoaded="OnModelLoaded"/>

Features

1-Transfom (translation,rotation,Scale)

2-Camera Control

Comming.. 

1-changing the material

2-changing the texture

3-controlling the light 
