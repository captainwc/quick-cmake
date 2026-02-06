#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <string>

#include "AddressBook.pb.h"
#include "skutils/logger.h"
#include "skutils/random.h"

void fillInformationForEachPerson(tutorial::book::Person* person) {
  person->set_id(RANDTOOL.getRandomInt(1000));
  person->set_email(RANDTOOL.getRandomEmail());
  person->set_name(RANDTOOL.getRandomName());
  auto phone_number = std::make_shared<tutorial::book::Person::PhoneNumber>();
  phone_number->set_number(RANDTOOL.getRandomPhoneNumber());
  person->set_allocated_rd_phone_number(phone_number.get());
  person->set_rd_phone_type(tutorial::book::Person::PhoneType(RANDTOOL.getRandomInt(0, 2)));
  person->mutable_rd_phone_number()->set_number(RANDTOOL.getRandomPhoneNumber());
  switch (person->random_attr_case()) {
    case tutorial::book::Person::kRdPhoneType:
    case tutorial::book::Person::kRdPhoneNumber:
    default: break;
  }
  //* mutable_xxx() means that you can modify it.
  (*person->mutable_name()).append(" ");

  int phoneNumPerPerson = 3;
  while (phoneNumPerPerson--) {
    //* repeated attr has method add_xxx()
    tutorial::book::Person::PhoneNumber* phone = person->add_phone();
    phone->set_number(RANDTOOL.getRandomPhoneNumber());
    auto type = RANDTOOL.getRandomInt(0, 2);
    if (type == 0) {
      phone->set_type(tutorial::book::Person::MOBILE);
    } else if (type == 1) {
      phone->set_type(tutorial::book::Person::HOME);
    } else {
      phone->set_type(tutorial::book::Person::WORK);
    }
  }
}

bool saveToFile(const tutorial::book::AddressBook& bookers, std::string filename) {
  std::fstream output(filename, std::ios::out | std::ios::trunc | std::ios::binary);
  if (!bookers.SerializePartialToOstream(&output)) {
    SK_ERROR("Failed to write address book.");
    return false;
  }
  return true;
}

auto parseFromFile(std::string filename) -> std::optional<tutorial::book::AddressBook> {
  std::fstream input(filename, std::ios::in | std::ios::binary);
  if (!input) {
    SK_ERROR("File \"{}\" Unexists.", filename);
    return std::nullopt;
  }
  tutorial::book::AddressBook ret;
  if (!ret.ParseFromIstream(&input)) {
    SK_ERROR("Parse information from {} FAILED.", filename);
    return std::nullopt;
  }
  return std::optional(ret);
}

int main() {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  const char* binFileName = "proto_data.bin";

  tutorial::book::AddressBook address_book;
  for (int i = 0; i < 3; i++) {
    fillInformationForEachPerson(address_book.add_person());
  }

  saveToFile(address_book, binFileName);

  auto reParse = parseFromFile(binFileName);
  DUMP(reParse.value_or(tutorial::book::AddressBook{}).DebugString());

  google::protobuf::ShutdownProtobufLibrary();

  return 0;
}
