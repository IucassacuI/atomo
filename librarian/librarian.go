package main

import (
	"fmt"
	"librarian/atom"
	"librarian/rss"
	"encoding/xml"
	"net/http"
	"io"
	"os"
	"flag"
	"strings"
	"crypto/sha256"
	"time"
)

func createlibrary(){
	if err := os.Chdir("library"); err != nil {
		os.Mkdir("library", os.ModePerm)
	}

	var dir string
	dir, err := os.Getwd()

	if err != nil {
		os.Exit(4)
	} else if !strings.Contains(dir, "library/"){
		os.Chdir("library")
	}
}

func calchash(url string) string {
	var hashbytes [32]byte = sha256.Sum256([]byte(url))
	var hash string = fmt.Sprintf("%x", hashbytes)

	return hash
}

func checklibrary(feedurl string) bool {
	var hash string = calchash(feedurl)

	_, err := os.ReadFile(hash)
	
	return err == nil
}

func readfromlibrary(feedurl string) []byte {
	var hash string = calchash(feedurl)

	content, _ := os.ReadFile(hash)

	return content
}

func updatelibrary(feed string, showstatus bool){
	var hash string = calchash(feed)

	resp, err := http.Get(feed)

	if err != nil {
		os.Exit(2)
	}

	defer resp.Body.Close()
	body, err := io.ReadAll(resp.Body)

	if err != nil {
		os.Exit(3)
	}

	if checklibrary(feed) && showstatus {
		onlibrary, _ := os.ReadFile(hash)
		fmt.Printf("%t\n", (len(onlibrary) != len(body)))
	}

	os.WriteFile(hash, body, os.ModePerm)
}

func showinfoatom(feed []byte, metadata bool, item int, items bool){
	var parsed atom.Feed
	err := xml.Unmarshal(feed, &parsed)

	if err != nil {
		os.Exit(5)
	}

	if metadata {
		fmt.Println(parsed.Title)
		fmt.Println(parsed.Author.Name)
		fmt.Println(parsed.Author.URI)
		fmt.Println(parsed.Published)
		fmt.Println(parsed.Updated)
	} else if item != -1 {
		entry := parsed.Entries[item]

		fmt.Println(entry.Title)
		fmt.Println(entry.Hyperlink.Href)
		fmt.Println(entry.Published)
		fmt.Println(entry.Updated)
	} else if  items {
		for _, elm := range(parsed.Entries){
			fmt.Println(elm.Title)
		}
	}
}

func showinforss(data []byte, metadata bool, item int, items bool){
	var parsed rss.Feed
	xml.Unmarshal(data, &parsed)
	var feed rss.Channel = parsed.Channel

	if metadata {
		fmt.Println(feed.Title)
		fmt.Println(feed.Copyright)
		fmt.Println(feed.Hyperlink)
		fmt.Println(feed.Published)
		fmt.Println(feed.Updated)
	} else if item != -1 {
		item := feed.Items[item]

		fmt.Println(item.Title)
		fmt.Println(item.Hyperlink)
		fmt.Println(item.Published)
		fmt.Println()
	} else if items {
		for _, elm := range(feed.Items){
			fmt.Println(elm.Title)
		}
	}
}

func main(){

	var feed = flag.String("feed", "", "")
	var update = flag.Bool("update", false, "")
	var metadata = flag.Bool("metadata", false, "")
	var item = flag.Int("item", -1, "")
	var items = flag.Bool("items", false, "")

	flag.Parse()
	
	if *feed == "" {
		os.Exit(1)
	}

	createlibrary()
	
	if *update {
		updatelibrary(*feed, true)
		time.Sleep(5 * time.Second)
	}

	var data = readfromlibrary(*feed)

	
	if strings.Contains(string(data), "<feed"){
		showinfoatom(data, *metadata, *item, *items)
	} else if strings.Contains(string(data), "<rss"){
		showinforss(data, *metadata, *item, *items)
	} else {
		os.Exit(1)
	}
}
