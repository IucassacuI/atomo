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

func updatelibrary(feed string){
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

	if checklibrary(feed){
		onlibrary, _ := os.ReadFile(hash)

		os.Chdir("..")

		var out, _ = os.OpenFile("out", os.O_CREATE|os.O_CREATE|os.O_WRONLY|os.O_APPEND, 0666)
		fmt.Fprintf(out, "%v", len(body) > len(onlibrary))
		out.Close()
		
		os.Chdir("library")
	}

	os.WriteFile(hash, body, os.ModePerm)	
}

func showinfoatom(feed []byte, metadata bool, item int, items bool){
	var parsed atom.Feed
	err := xml.Unmarshal(feed, &parsed)

	if err != nil {
		os.Exit(5)
	}

	for _, elm := range([]*string{&parsed.Title, &parsed.Author.Name, &parsed.Author.URI, &parsed.Published, &parsed.Updated}){
		if *elm == "" {
			*elm = "N/A"
		}
	}

	var out, _ = os.OpenFile("out", os.O_CREATE|os.O_APPEND|os.O_WRONLY, 0666)	
	
	if metadata {
		fmt.Fprintln(out, parsed.Title)
		fmt.Fprintln(out, parsed.Author.Name)
		fmt.Fprintln(out, parsed.Author.URI)
		fmt.Fprintln(out, parsed.Published)
		fmt.Fprintln(out, parsed.Updated)
	} else if item != -1 {
		entry := parsed.Entries[item]

		for _, elm := range([]string{entry.Title, entry.Hyperlink.Href, entry.Published, entry.Updated}){
			if elm == "" {
				fmt.Fprintln(out, "N/A")
			} else {
				fmt.Fprintln(out, elm)
			}
		}
		
	} else if  items {
		for _, elm := range(parsed.Entries){
			fmt.Fprintln(out, elm.Title)
		}
	}

	out.Close()
}

func showinforss(data []byte, metadata bool, item int, items bool){
	var parsed rss.Feed
	err := xml.Unmarshal(data, &parsed)

	if err != nil {
		os.Exit(5)
	}

	var feed rss.Channel = parsed.Channel 
	for _, elm := range([]*string{&feed.Title, &feed.Copyright, &feed.Hyperlink, &feed.Published, &feed.Updated}){
		if *elm == "" {
			*elm = "N/A"
		}
	}

	var out, _ = os.OpenFile("out", os.O_CREATE|os.O_APPEND|os.O_WRONLY, 0666)

	if metadata {
		fmt.Fprintln(out, feed.Title)
		fmt.Fprintln(out, feed.Copyright)
		fmt.Fprintln(out, feed.Hyperlink)
		fmt.Fprintln(out, feed.Published)
		fmt.Fprintln(out, feed.Updated)
	} else if item != -1 {
		item := feed.Items[item]

		for _, elm := range([]string{item.Title, item.Hyperlink, item.Published}){
			if elm == "" {
				fmt.Fprintln(out, "N/A")
			} else {
				fmt.Fprintln(out, elm)
			}
		}
		fmt.Fprintln(out, "N/A")
	} else if items {
		for _, elm := range(feed.Items){
			fmt.Fprintln(out, elm.Title)
		}
	}

	out.Close()
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
		updatelibrary(*feed)
	}

	var data = readfromlibrary(*feed)
	os.Chdir("..")
	
	if strings.Contains(string(data), "<feed"){
		showinfoatom(data, *metadata, *item, *items)
	} else if strings.Contains(string(data), "<rss"){
		showinforss(data, *metadata, *item, *items)
	} else {
		os.Exit(1)
	}
}
