package main

import (
	//"log"
	"net/http"
	//"text/template"
	"fmt"
	"os"
)

func index_page(w http.ResponseWriter, r *http.Request) {
	pwd, _ := os.Getwd()

	fmt.Println(pwd)
	http.ServeFile(w, r, pwd+"/html/demo.html")
}

func main() {
	//http.HandleFunc("d", handler)
	pwd, _ := os.Getwd()
	http.HandleFunc("/", index_page)
	http.Handle("/get_image", http.FileServer(http.Dir(pwd+"/sendables")))
	http.ListenAndServe(":8080", nil)
}
