15日目
=====

## やったこと

- リフレッシュレートを固定した

  今まではイベントキューの中身が処理し終わったら`refresh_screen`を呼ぶようにしていたが、これをタイマで定期的に呼ぶようにした。
  若干性能が上がった。

- タイマイベントをイベントキューに載せないようにした

  今後生のタイマを使うことはあんまり無さそうなのと、キューを気にしないスレッドを作りたかったから。
