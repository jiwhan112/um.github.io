export type ThinkingPairEntry={number:string;chapter:number;path:string;study_date:string;published_date:string;topic_en:string};
export function validateThinkingPairs(catalog:{episodes:ThinkingPairEntry[];current:{number:string;study_date:string};next:{number:string;study_date:string}},policy:{publicationUnit:string;pagesPerTopic:number;limit:string}):{topics:number;pages:number};
