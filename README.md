# Selfe Printing Camera for SPRESENSE

Spresenseのカメラサンプルソースコードを参考に製作しました。MFKyoto2019でのデモになります。

# 処理
カメラのPreview画像をLCDに表示し、シャッターが押されるとPreview画像をグレースケールに変換、誤差拡散法により2値化したイメージをサーマルプリンタに出力します。
チルト機能とは自撮り用のLCD表示を回転させる機能です。

[Spresense + Spresense 拡張ボード + Spresenseカメラボード](https://developer.sony.com/ja/develop/spresense/)  
[AS-289R2 Thermal Printer Shield](http://www.nada.co.jp/as289r2/)  

![Selfe Printing Camera for SPRESENSE](https://github.com/NADA-ELECTRONICS/Selfe-Printing-Camera-for-SPRESENSE/blob/master/DSC_0117.JPG)

# 配線
Spresense pin1 -- AS-289R2(RxD)  
Spresense pin2 -- シャッタースイッチ  
Spresense pin4 -- リミットスイッチ(チルト機能)  
Spresense pin8 -- TFT_RST(ili9341)  
Spresense pin9 -- TFT_DC(ili9341)  
Spresense pin10 -- TFT_CS(ili9341)  
Spresense pin11 -- TFT_MOSI(ili9341)  
Spresense pin12 -- TFT_MISO(ili9341)  
Spresense pin13 -- TFT_CLK(ili9341)  

