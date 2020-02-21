・ファイル構成

log : バッチファイルから実行したマクロの実行ログが保存されます。
ttl
 - comb : 組み合わせ項目のマクロ群が保存されています。
 - comb_refacter : 組み合わせ項目(リファクタリング変更部)のマクロ群が保存されています。
 - other: 組み合わせ以外のマクロ群(各状態のAPI実行・CB)が保存されています。
 - comport.ttl : マクロ実行に必要な設定を保存するファイルです。（COMポート, APN設定, PINコード)
comb_refacter.bat ; comb_refacterフォルダ内のマクロを実行します
comb.bat ; combフォルダ内のマクロを実行します
other_test.bat : otherフォルダ内のマクロを実行します
readme.txt : これ

以下実行マクロ

・comb_test.bat
	lteapi_test_comb01.ttl
	lteapi_test_comb02.ttl
	lteapi_test_comb03.ttl
	lteapi_test_comb04(1160_only).ttl
	lteapi_test_comb05.ttl
	lteapi_test_comb06.ttl
	lteapi_test_comb07.ttl
	lteapi_test_comb08(1160_only).ttl
	lteapi_test_comb09(1160_only).ttl
	lteapi_test_comb10.ttl
	lteapi_test_comb11.ttl
	lteapi_test_comb12(1160_only).ttl
	lteapi_test_comb13.ttl
	lteapi_test_comb15.ttl

・other_test.bat
	lteapi_test_api_state_dton.ttl
	lteapi_test_api_state_atch(1160_only).ttl
	lteapi_test_api_state_pwron.ttl
	lteapi_test_api_strpntst_atch(1160_only).ttl
	lteapi_test_api_strpntst_pwron.ttl
	lteapi_test_cb_edrx.ttl
	lteapi_test_cb_psm.ttl
	lteapi_test_cb_waitpin.ttl
	
