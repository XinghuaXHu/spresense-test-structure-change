test/sqa/combination/step_audio_lte
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Description:
  AESM + LTE + Audio combination test application.

Build:
tools/config.py examples/step_counter examples/audio_player examples/lte_http_get -m

> Examples
[ ] Audio player example
[ ] HTTP GET method using LTE example
[ ] Step counter sensor example

> Test > SQA > Combination
[*] Step counter and Audio and LTE combination test

Run:
step_audio_lte [OPTIONS] HOST PORT PATH

ex)step_audio_lte -H "Content-Type: application/json" 192.168.100.104 80 /

Options:
 -H <header> : Http header string
