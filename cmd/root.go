package cmd

import (
	"fmt"
	"io/ioutil"
	"net/http"
	"net/url"
	"os"
	"strings"
	"time"

	log "github.com/sirupsen/logrus"
	"github.com/spf13/cobra"
)

var serverName, messageText, receiverNumber, port, sendsmsURL string
var verbose, checkSentStatus bool

func init() {
	//	cobra.OnInitialize(initConfig)
	rootCmd.PersistentFlags().StringVarP(&serverName, "servername", "s", "", "Server hosting smstools")
	rootCmd.PersistentFlags().StringVarP(&messageText, "messagetext", "t", "", "")
	rootCmd.PersistentFlags().StringVarP(&port, "port", "p", "80", "Port Number")
	rootCmd.PersistentFlags().StringVarP(&receiverNumber, "receivernumber", "b", "", "Receiver number")
	// rootCmd.PersistentFlags().BoolVarP(&verbose, "verbose", "v", false, "verbose output")
	rootCmd.PersistentFlags().BoolVarP(&checkSentStatus, "checksentstatus", "c", false, "verbose output")
	// setting flags with MarkPersistentFlagRequired makes easier handling or error conditions
	rootCmd.MarkPersistentFlagRequired("messagetext")
	rootCmd.MarkPersistentFlagRequired("servername")
	rootCmd.MarkPersistentFlagRequired("receivernumber")
}

// func initConfig() {
// 	//
// }

var rootCmd = &cobra.Command{
	Use:   "sendsms",
	Short: "sending sms from command line trough smstools3 server implementation",
	Run: func(cmd *cobra.Command, args []string) {
		var netClient = &http.Client{
			Timeout: time.Second * 10,
		}

		sendsmsURL := "http://" + serverName + ":" + port + "/sendsms.php"
		dataPostMain := url.Values{}
		dataPostMain.Set("sent", "cli")
		dataPostMain.Add("fromform", "true")
		dataPostMain.Add("to", receiverNumber)
		dataPostMain.Add("text", messageText)
		if checkSentStatus == true {
			dataPostMain.Add("confirmation", "on")
		}
		r, _ := http.NewRequest("POST", sendsmsURL, strings.NewReader(dataPostMain.Encode()))
		r.Header.Add("Content-Type", "application/x-www-form-urlencoded")
		resp, err := netClient.Do(r)
		if err != nil {
			log.Error("Server responded with error", err)
		}
		defer resp.Body.Close()
		mainBody, _ := ioutil.ReadAll(resp.Body)
		mainText := string(mainBody)
		if checkSentStatus == true {
			for i := 0; i < 60; i++ {
				dataPostCheck := url.Values{}
				dataPostCheck.Add("sent", "cli")
				dataPostCheck.Add("confirmation", "on")
				dataPostCheck.Add("msgid", mainText[strings.Index(mainText, "#")+1:])
				r1, _ := http.NewRequest("POST", sendsmsURL, strings.NewReader(dataPostCheck.Encode()))
				r1.Header.Add("Content-Type", "application/x-www-form-urlencoded")
				resp1, err := netClient.Do(r1)
				if err != nil {
					log.Error("Server responded with error", err)
					defer resp1.Body.Close()
				}
				checkBody, _ := ioutil.ReadAll(resp1.Body)
				checkText := string(checkBody)
				if strings.Contains(checkText, "FAILED") == true {
					log.Fatal("Sending failed: ", checkText)
				} else if strings.Contains(checkText, "SENT") == true {
					log.Info("Message Sent: ", checkText)
					break
				}
				time.Sleep(time.Second)
			}
		} else {
			log.Info("Message Sent: ", mainText)
		}
		// if verbose == true {
		// 	log.WithFields(log.Fields{
		// 		"serverName":      serverName,
		// 		"messageText":     messageText,
		// 		"receiverNumber":  receiverNumber,
		// 		"verbose":         verbose,
		// 		"checkSentStatus": checkSentStatus,
		// 		"sendsmsURL":      sendsmsURL,
		// 	}).Warn("Configured command flags")
		// }

	},
}

// Execute function of command line argument parsing
func Execute() {
	if err := rootCmd.Execute(); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
}
