VC_Basic/Ex.cpp(Device Art Toolkitの参考プログラム)の75行目
SetData関数内, ビット単位でポートAのPIOを制御する部分のプログラムに誤りがあります。

誤(現行): pModule->PioBoardA.Bit.P1 = ~Module00.PioBoardA.Bit.P1;
正:       pModule->PioBoardA.Bit.P1 = ~pModule->PioBoardA.Bit.P1;

関数で引数としてとっているアドレスではなく、グローバルの値を参照してしまっている不具合です。
このプロジェクトを使うときには修正して使ってください。


CM001_Basicフォルダ内にあるプログラムはDeviceArtToolkit CM001基盤のdsPic30f4011に書き込まれているプログラムです。
基本的に弄る必要はありません。

SC02フォルダ内にあるプログラムはDeviceArtToolkit SC02基盤のdsPic30f4011に書き込まれているプログラムです。
基本的に弄る必要はありません。

VC_Basicフォルダ内にあるEx.cppを参考にプログラムを書いてください。