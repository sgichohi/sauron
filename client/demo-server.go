package main

import (
	//"log"
	"net/http"
	//"text/template"
	"fmt"
	"log"
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
	fmt.Println(pwd)
	fmt.Println("Here")
	sendable := pwd + "/sendables/"
	fmt.Println(sendable)
	log.Fatal(http.ListenAndServe(":8080", http.FileServer(http.Dir(sendable))))
	//http.HandleFunc("/", index_page)

	//http.ListenAndServe(":8080", nil)
}
