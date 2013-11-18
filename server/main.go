
package main

import (
        zmq "github.com/pebbe/zmq3"
        "time"
        "fmt"
        "os"       
)

func save_to_file(frame []byte) {

        name := fmt.Sprintf("%d", time.Now().UnixNano())
        
                rand_file, err := os.Create(name)
                defer rand_file.Close()
                if err != nil {
                        return
                }
                _, werr := rand_file.Write(frame)
                if werr != nil {
                        return 
                }
}

func main() {
        
        server, _ := zmq.NewSocket(zmq.REP)
        defer server.Close()
        server.Bind("tcp://*:5000")
        fmt.Println("Server is up and running!")
        for ; ; {
                request, _ := server.RecvBytes(0)
                fmt.Printf("I: normal request (%s)\n", request)
                go save_to_file(request)
                server.SendMessage("200")
        }
}