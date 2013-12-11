package main

import (
	//"log"
	"net/http"
	//"text/template"
	"encoding/json"
	"fmt"
	"log"
	"os"
	"path/filepath"
	"sort"
	//"strings"
)

func index_page(w http.ResponseWriter, r *http.Request, static string) {
	http.ServeFile(w, r, static+"/html/demo.html")
}
func find_images(w http.ResponseWriter, r *http.Request, images string) {
	fmt.Println("Finding images")
	search_path := images + "*/*.jpg"
	m := make(map[string]string)
	//fmt.Println(search_path)
	files, _ := filepath.Glob(search_path)
	sort.Sort(sort.Reverse(sort.StringSlice(files)))

	i := 0

	for _, fle := range files {
		//fmt.Println(fle)
		rel, err := filepath.Rel(images, fle)
		if err != nil {
			panic("oops no relative")
		}
		m[string(i)] = "/images/" + rel
		i += 1
		if i > 5 {
			break
		}
	}
	//fmt.Println(m)

	//fmt.Println(m)
	data, _ := json.Marshal(m)
	w.Header().Set("Content-Type", "application/json")
	w.Write(data)
	//fmt.Println(data)

}

func main() {
	//http.HandleFunc("d", handler)
	//fmt.Println(len(os.Args), os.Args)
	static, _ := os.Getwd()
	fmt.Println("Serving static files from")
	fmt.Println(static)
	fmt.Println("Serving images files from")
	images := os.Args[1]
	fmt.Println(images)

	http.HandleFunc("/image_list", func(w http.ResponseWriter, r *http.Request) {
		find_images(w, r, images)
	})
	http.HandleFunc("/", func(w http.ResponseWriter, r *http.Request) {
		index_page(w, r, static)
	})
	http.Handle("/static/", http.StripPrefix("/static/", http.FileServer(http.Dir(static))))
	http.Handle("/images/", http.StripPrefix("/images/", http.FileServer(http.Dir(images))))
	log.Fatal(http.ListenAndServe(":8080", nil))
	//http.HandleFunc("/", index_page)

	//http.ListenAndServe(":8080", nil)
}
