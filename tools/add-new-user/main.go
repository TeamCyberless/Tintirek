package main

import (
	"crypto/rand"
	"crypto/sha256"
	"database/sql"
	"encoding/base64"
	"flag"
	"fmt"
	"log"
	"time"

	_ "github.com/mattn/go-sqlite3"
)

func SHA256(val string) (string, error) {
	hasher := sha256.New()

	_, err := hasher.Write([]byte(val))
	if err != nil {
		return "", err
	}

	hashBytes := hasher.Sum(nil)
	hashStr := fmt.Sprintf("%x", hashBytes)

	return hashStr, nil
}

func main() {
	userDbPath := flag.String("userDbPath", "./user.db", "User database file path")
	username := flag.String("username", "exampleuser", "Username")
	fullname := flag.String("fullname", "Example User", "Username")
	email := flag.String("email", "your@mail.com", "Username")
	password := flag.String("password", "123123", "Password without encryption")
	iterationcount := flag.Int("iterationcount", 3, "Iteration count for salting")
	salt := flag.String("salt", "", "Salt value. If no value is given, it will be generated")
	flag.Parse()

	saltVal := *salt
	if saltVal == "" {
		bytes := make([]byte, 12)
		_, err := rand.Read(bytes)
		if err != nil {
			fmt.Println(err)
			return
		}

		randomString := base64.StdEncoding.EncodeToString(bytes)
		fmt.Println("random salt:", randomString)
		saltVal = randomString
	}

	passwd, err := SHA256(*password)
	if err != nil {
		fmt.Println(err)
		return
	}
	fmt.Println("Passwd hashed:", passwd)
	for i := 0; i < *iterationcount; i++ {
		fmt.Printf("Passwd hash info %d: %s\n", i, passwd + "::" + saltVal)
		passwd, err = SHA256(passwd + "::" + saltVal)
		if err != nil {
			fmt.Println(err)
			return
		}
		fmt.Printf("Passwd hashed with salt %d: %s\n", i, passwd)
	}
	fmt.Println("Passwd:", passwd)

	fmt.Println("db:", *userDbPath)
	db, err := sql.Open("sqlite3", *userDbPath)
	if err != nil {
		fmt.Println(err)
		return
	}
	defer db.Close()

	tx, err := db.Begin()
	if err != nil {
		fmt.Println(err)
		return
	}

	stmt, err := tx.Prepare("INSERT INTO user(username, fullname, email, password_hash, salt, iteration) values(?, ?, ?, ?, ?, ?)")
	if err != nil {
		fmt.Println(err)
		return
	}
	defer stmt.Close()

	result, err := stmt.Exec(*username, *fullname, *email, passwd, saltVal, *iterationcount)
	if err != nil {
		fmt.Println(err)
	} else {
		err = tx.Commit()
		if err != nil {
			fmt.Println(err)
		} else {
			val, err := result.LastInsertId()
			if err != nil {
				fmt.Println(err)
				return
			}
			fmt.Println("Done.", val)
		}

		query := "SELECT * FROM user"
		rows, err := db.Query(query)
		if err != nil {
			log.Fatal(err)
			return
		}
		defer rows.Close()

		fmt.Println("Details:")
		for rows.Next() {
			var id, iteration int
			var username, fullname, email, password_hash, salt string
			var ticket sql.NullString
			var created, updated time.Time

			err := rows.Scan(&id, &username, &fullname, &email, &ticket, &password_hash, &salt, &iteration, &created, &updated)
			if err != nil {
				log.Fatal(err)
			}

			fmt.Printf("ID: %d\nUsername: %s\nFull Name: %s\nEmail: %s\nTicket: %s\nHashed Passwd: %s\nSalt: %s\nIteration: %d\nCreated: %s\nUpdated: %s\n", id, username, fullname, email, ticket.String, password_hash, salt, iteration, created.String(), updated.String())
		}
	}
}
