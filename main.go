package main

import (
	"bytes"
	"fmt"
	"github.com/bytedance/sonic"
)

func main() {
	var o1 = map[string]interface{}{
		"a": "b",
	}
	var o2 = 1
	var w = bytes.NewBuffer(nil)
	var enc = sonic.ConfigDefault.NewEncoder(w)
	err := enc.Encode(o1)
	if err != nil {
		return
	}
	err = enc.Encode(o2)
	if err != nil {
		return
	}
	fmt.Println(w.String())
}
